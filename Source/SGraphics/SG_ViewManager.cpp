#include "SGraphics_Precompiled.h"
#include "SG_ViewManager.h"
#include "SG_View.h"

namespace Shift
{
	SG_ViewManager::SG_ViewManager()
	{
	}
	SG_ViewManager::~SG_ViewManager()
	{
	}
	void SG_ViewManager::AddView(SG_View* aView)
	{
		myActiveViews.Add(aView);
	}
	void SG_ViewManager::RemoveView(SG_View* aView)
	{
		myActiveViews.RemoveCyclic(aView);
	}
	void SG_ViewManager::Update()
	{
		const uint activeViews = myActiveViews.Count();

		for (uint i = 0; i < activeViews; ++i)
		{
			SG_View* view = myActiveViews[i];
			view->Prepare();
		}

		for (uint i = 0; i < activeViews; ++i)
		{
			myActiveViews[i]->Render();
		}
	}
}