#pragma once
#include <iterator>

namespace LE
{
template <class ElementType>
class IntrusiveLinkedList
{
public:
	class Iterator : public std::iterator<std::forward_iterator_tag, IntrusiveLinkedList, std::ptrdiff_t, IntrusiveLinkedList*,
	                                      IntrusiveLinkedList&>
	{
	public:
		Iterator()
			: CurrentLink(nullptr)
		{
		}

		explicit Iterator(ElementType* InLink)
			: CurrentLink(InLink)
		{}

		IntrusiveLinkedList& operator*() const { return *CurrentLink; }
		IntrusiveLinkedList* operator->() const { return CurrentLink; }

		void Next()
		{
			CurrentLink = CurrentLink->GetNextElement();
		}

		Iterator& operator++()
		{
			Next();
			return *this;
		}

		Iterator& operator++(int)
		{
			Iterator temp = *this;
			Next();
			return temp;
		}

		friend bool operator==(const Iterator& First, const Iterator& Second)
		{
			return First.CurrentLink == Second.CurrentLink;
		}

		friend bool operator!=(const Iterator& First, const Iterator& Second)
		{
			return First.CurrentLink != Second.CurrentLink;
		}

	private:
		ElementType* CurrentLink;
	};

	IntrusiveLinkedList()
		: NextElement(nullptr)
		  , PrevElementNextLink(nullptr)
	{
	}

	void Unlink()
	{
		if (NextElement)
		{
			NextElement->PrevElementNextLink = PrevElementNextLink;
		}

		if (PrevElementNextLink)
		{
			*PrevElementNextLink = NextElement;
		}

		NextElement = nullptr;
		PrevElementNextLink = nullptr;
	}

	void LinkBeforeThis(ElementType* Element)
	{
		PrevElementNextLink = Element->PrevElementNextLink;
		Element->PrevElementNextLink = &NextElement;

		NextElement = Element;

		if (PrevElementNextLink)
		{
			*PrevElementNextLink = static_cast<ElementType*>(this);
		}
	}

	void LinkAfterThis(ElementType* Element)
	{
		NextElement = Element->NextElement;
		Element->NextElement = static_cast<ElementType*>(this);;

		PrevElementNextLink = &Element->NextElement;

		if (NextElement)
		{
			NextElement->PrevElementNextLink = &NextElement;
		}
	}

	void ReplaceThisLink(ElementType* Element)
	{
		ElementType*& replaceNextElement = Element->NextElement;
		ElementType**& replacePrevElementNextLink = Element->PrevElementNextLink;

		NextElement = replaceNextElement;
		PrevElementNextLink = replacePrevElementNextLink;

		if (!PrevElementNextLink)
		{
			*PrevElementNextLink = static_cast<ElementType*>(this);
		}

		if (!NextElement)
		{
			NextElement->PrevElementNextLink = &NextElement;
		}

		replaceNextElement = nullptr;
		replacePrevElementNextLink = nullptr;
	}

	void LinkAsHead(ElementType*& Head)
	{
		if (!Head)
		{
			Head->PrevElementNextLink = &NextElement;
		}

		NextElement = Head;
		PrevElementNextLink = &Head;
		Head = static_cast<ElementType*>(this);
	}

	bool IsLinked() const
	{
		return PrevElementNextLink != nullptr;
	}

	ElementType** GetPrevElementNextLink() const
	{
		return PrevElementNextLink;
	}

	ElementType* GetNextElement() const
	{
		return NextElement;
	}

private:
	ElementType* NextElement;
	ElementType** PrevElementNextLink;
};
}
