#pragma once
#include "SC_Vector.h"
#include "SC_GrowingArray.h"
#include "SC_Array.h"
#include "SC_AABB.h"
#include "SC_VolumeObject.h"

namespace Shift
{
	template <class Type>
	class SC_QuadTree
	{
	public:
		struct Cell
		{
			Cell(const SC_QuadTree<Type>* aParent)
				: myParent(aParent)
			{
				SC_Fill<Cell*>(myChildren.GetBuffer(), 4, nullptr);
			}

			~Cell()
			{
				for (uint i = 0; i < 4; ++i)
					delete myChildren[i];
			}

			SC_GrowingArray<SC_VolumeObject<Type>> myObjects;
			SC_Array<Cell*, 4> myChildren;
			SC_AABB myAABB;
			const SC_QuadTree<Type>* myParent;

			bool IsInside(const SC_VolumeObject<Type>& aObject) const;
			bool Insert(const SC_VolumeObject<Type>& aObject);
			void SplitCell();
		};

		SC_QuadTree(const SC_Vector2f& aSize, const uint aMaxObjectsPerCell, const uint aLoosenessFactor = 1);
		~SC_QuadTree();

		void Add(const SC_VolumeObject<Type>& aObject);
		bool Clear();

		const Type& Find(const SC_VolumeObject<Type>& aObject);
		bool GetObjectsAtPoint(const SC_Vector2f& aPoint, SC_GrowingArray<SC_VolumeObject<Type>>& aDataOut);

		const uint myMaxObjectsPerCell;
		const uint myLoosenessFactor;
		const SC_Vector2f mySize;

	private:
		Cell myInitialVolume;
	};

	template<class Type>
	inline bool SC_QuadTree<Type>::Cell::IsInside(const SC_VolumeObject<Type>& aObject) const
	{
		bool isInside = myAABB.IsInside(SC_Vector3f(aObject.myPoint.x, 0.f, aObject.myPoint.z));
		return isInside;
	}

	template<class Type>
	inline bool SC_QuadTree<Type>::Cell::Insert(const SC_VolumeObject<Type>& aObject)
	{
		if (!IsInside(aObject))
			return false;

		for (uint i = 0; i < 4; ++i)
		{
			Cell* child = myChildren[i];
			if (child && child->Insert(aObject))
				return true;
		}

		if (myObjects.Count() < myParent->myMaxObjectsPerCell)
		{
			myObjects.Add(aObject);

			if (myObjects.Count() == myParent->myMaxObjectsPerCell)
				SplitCell();

			return true;
		}

		return false;
	}

	template<class Type>
	inline void SC_QuadTree<Type>::Cell::SplitCell()
	{
		float toCenter((myAABB.myMax - myAABB.myMin).Length() * 0.5f);
		SC_Vector3f center = myAABB.myMin + toCenter;
		SC_Vector3f newSize(toCenter, 0.0f, toCenter);

		SC_AABB split[4];

		// bottom left
		split[0].myMin = SC_Vector3f(center.x - newSize.x, 0.0f, center.z - newSize.z);
		split[0].myMax = SC_Vector3f(center.x, 0.0f, center.z);

		// top left
		split[1].myMin = SC_Vector3f(center.x - newSize.x, 0.0f, center.z);
		split[1].myMax = SC_Vector3f(center.x, 0.0f, center.z + newSize.z);

		// top right
		split[2].myMin = SC_Vector3f(center.x, 0.0f, center.z);
		split[2].myMax = SC_Vector3f(center.x + newSize.x, 0.0f, center.z + newSize.z);

		// bottom right
		split[3].myMin = SC_Vector3f(center.x, 0.0f, center.z - newSize.z);
		split[3].myMax = SC_Vector3f(center.x + newSize.x, 0.0f, center.z);

		for (uint i = 0; i < 4; ++i)
		{
			myChildren[i] = new Cell(myParent);
			myChildren[i]->myAABB = split[i];
		}

		for (SC_VolumeObject<Type>& object : myObjects)
			Insert(object);

		myObjects.RemoveAll();
	}

	template<class Type>
	inline SC_QuadTree<Type>::SC_QuadTree(const SC_Vector2f& aSize, const uint aMaxObjectsPerCell, const uint aLoosenessFactor)
		: myInitialVolume(this)
		, mySize(aSize)
		, myMaxObjectsPerCell(aMaxObjectsPerCell)
		, myLoosenessFactor(aLoosenessFactor)
	{
		const SC_Vector2f halfSize = { mySize.x * 0.5f, mySize.y * 0.5f };
		myInitialVolume.myAABB.myMin = SC_Vector3f(-halfSize.x, 0.0f, -halfSize.z);
		myInitialVolume.myAABB.myMax = SC_Vector3f(halfSize.x, 0.0f, halfSize.z);
	}

	template<class Type>
	inline SC_QuadTree<Type>::~SC_QuadTree()
	{
	}

	template<class Type>
	inline void SC_QuadTree<Type>::Add(const SC_VolumeObject<Type>& aObject)
	{
		if (!myInitialVolume.Insert(aObject))
			assert("QuadTree is full.");
	}
}