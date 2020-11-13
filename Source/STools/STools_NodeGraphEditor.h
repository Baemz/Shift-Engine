#pragma once

namespace Shift 
{
	class STools_NodeGraphEditor
	{
	public:
		STools_NodeGraphEditor();
		virtual ~STools_NodeGraphEditor();

		virtual void Open() = 0;
		virtual void Close() = 0;

	protected:
		virtual void LoadGraph() = 0;
	};
}
