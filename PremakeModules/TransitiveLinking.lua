-- TransitiveLinking.lua
-- TransitiveLinking Premake module

print("Transitive Linking is loaded")

local projectMetadata = {}

function publicIncludeDirs(includeDirs)
	local prj = project()
	prj.publicIncludeDirs = prj.publicIncludeDirs or {}

	for _, dir in ipairs(includeDirs) do
		table.insert(prj.publicIncludeDirs, dir)
	end

	includedirs(includeDirs)
end

function privateIncludeDirs(includeDirs)
	includedirs(includeDirs)
end

function register_project(project, projectPath)
	local resolvedIncludes = {}
	for _, dir in ipairs(project.publicIncludeDirs or {}) do
		table.insert(resolvedIncludes, path.getabsolute(path.join(projectPath, dir)))
	end

	projectMetadata[project.name] = {
		includeDirs = resolvedIncludes,
		links = project.links or {}
	}

	print (string.format("Module: %s with path: %s is registered", project.name, projectPath))
end

local function process_module(moduleName, wasVisited)
	wasVisited = wasVisited or {}
	if wasVisited[moduleName] then return {}, {} end

	wasVisited[moduleName] = true

	local metadata = projectMetadata[moduleName]
	if not metadata then return {}, {} end

	local includes = table.deepcopy(metadata.includeDirs or {})
	local links = table.deepcopy(metadata.links or {})

	local includeCount = #includes
	local linkCount = #links

	for _, link in ipairs(metadata.links or {}) do
		local subInclude, subLink = process_module(link, wasVisited)
		for _, it in ipairs(subInclude or {}) do 
			table.insert(includes, it)
			includeCount = includeCount + 1 
		end
		for _, it in ipairs(subLink or {}) do 
			table.insert(link, it) 
			linkCount = linkCount + 1 
		end
	end

	print(string.format("	Processed module: %s. Includes: %d, Transitive Links: %d", moduleName, includeCount, linkCount))

	return includes, links
end

function use_modules(moduleNames)
	links (moduleNames)

	_G.__deferred_use_modules = _G.__deferred_use_modules or {}
	table.insert(_G.__deferred_use_modules, {
		project = project().name,
		modules = moduleNames
	})

	print(string.format("	Module: %s is registered for transitive linking", project().name))
end

function link_modules()
	for _, entry in ipairs(_G.__deferred_use_modules) do
		print(string.format("Linking transitive includes for module: %s", entry.project))
		
		project(entry.project)
		
		for _, module in ipairs(entry.modules) do
			local includes, transitiveLinks = process_module(module)
			
			includedirs { includes }
			links { transitiveLinks }
		end
	end
end