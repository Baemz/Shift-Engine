#pragma once
namespace Shift
{
	class STools_NodePin
	{
	public:
		STools_NodePin();
		~STools_NodePin();
	};


	class STools_Node
	{
	public:
		STools_Node();
		virtual ~STools_Node();

		virtual void Load();
		virtual void Execute();

		virtual bool IsConst() const { return false; };
		virtual bool NeedsUpdate() const { return true; }

		const uint GetID() const { return myId; }
	protected:
		SC_GrowingArray<STools_NodePin*> myPins;

		uint myId;
	};
}
