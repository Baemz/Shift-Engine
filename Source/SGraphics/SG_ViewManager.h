#pragma once

namespace Shift
{
	class SG_View;
	class SG_ViewManager
	{
	public:
		SG_ViewManager();
		~SG_ViewManager();

		// Adds a view to the active list. (DOES NOT TAKE OWNERSHIP)
		void AddView(SG_View* aView);
		void RemoveView(SG_View* aView);

		void Update();

	private:
		SC_GrowingArray<SG_View*> myActiveViews;
	};
}
