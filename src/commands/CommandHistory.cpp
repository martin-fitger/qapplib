/*
Copyright XMN Software AB 2023

QAppLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. The GNU Lesser General Public License
is intended to guarantee your freedom to share and change all versions
of a program--to make sure it remains free software for all its users.

QAppLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QAppLib. If not, see <http://www.gnu.org/licenses/>.
*/

#include <qapplib/Debug.h>
#include <qapplib/commands/CommandHistory.hpp>

//#define DEBUG_ENABLED

namespace qapp
{
	const CCommandHistory::SCommand* CCommandHistory::NO_CLEAN_POINT = (CCommandHistory::SCommand*)(intptr_t)-1;

	CCommandHistory::CCommandHistory(IEditor& editor, CPagePool& page_pool)
		: m_Editor(editor)
		, m_DataBuffer(page_pool)
		, m_Allocator(page_pool)
	{
	}

	CCommandHistory::~CCommandHistory()
	{
		FreeAll();
	}

	void CCommandHistory::Clear()
	{
		FreeAll();
		m_CleanPoint = nullptr;
		OnModified();
	}

	bool CCommandHistory::CanUndo() const
	{
		return nullptr != m_UndoStack;
	}

	bool CCommandHistory::CanRedo() const
	{
		return nullptr != m_RedoStack;
	}
	
	void CCommandHistory::Undo()
	{
		if (!CanUndo())
		{
			QAPP_ASSERT(false && "Trying to undo with empty undo stack");
			return;
		}

		for (;;)
		{
			auto* cmd = m_UndoStack;
			m_UndoStack = m_UndoStack->m_Next;
			cmd->m_Next = m_RedoStack;
			m_RedoStack = cmd;
			Undo(*cmd);
			if (!m_UndoStack || !m_UndoStack->m_Incomplete)
				break;
		}

		OnModified();
	}

	void CCommandHistory::Redo()
	{
		if (!CanRedo())
		{
			QAPP_ASSERT(false && "Trying to redo with empty redo stack");
			return;
		}

		for(;;)
		{
			auto* cmd = m_RedoStack;
			m_RedoStack = m_RedoStack->m_Next;
			cmd->m_Next = m_UndoStack;
			m_UndoStack = cmd;
			Do(*cmd);
			if (!m_RedoStack || !cmd->m_Incomplete)
				break;
		}

		OnModified();
	}

	void CCommandHistory::ClearRedoStack()
	{
		if (!m_RedoStack)
			return;
		FreeCommandList(m_RedoStack);
		m_Allocator.restore(m_RedoStack->m_AllocatorState);
		m_RedoStack = nullptr;
		m_DataBuffer.resize(m_DataBufferPos);
		OnModified();
	}

	void CCommandHistory::SetCleanAtCurrentPosition()
	{
		if (m_CleanPoint == m_UndoStack)
		{
			return;
		}
		m_CleanPoint = m_UndoStack;
		OnModified();
	}

	void CCommandHistory::Do(SCommand& cmd)
	{
		page_buffer_istream data(m_DataBuffer, m_DataBufferPos, m_DataBufferPos + cmd.m_DataSize);
		m_DataBufferPos += cmd.m_DataSize;
		SCommandExecutionContext ctx_exec(m_Editor, data);
		cmd.m_Command->Do(ctx_exec);
		#ifdef DEBUG_ENABLED
				LOG_INFO("Command executed (size: %zu kB, total: %zu kB)", cmd.m_DataSize / 1024, m_DataBufferPos / 1024);
		#endif
	}

	void CCommandHistory::Undo(SCommand& cmd)
	{
		m_DataBufferPos -= cmd.m_DataSize;
		page_buffer_istream data(m_DataBuffer, m_DataBufferPos, m_DataBufferPos + cmd.m_DataSize);
		SCommandExecutionContext ctx_exec(m_Editor, data);
		cmd.m_Command->Undo(ctx_exec);
	}

	void CCommandHistory::FreeCommandList(SCommand* front)
	{
		while (front)
		{
			auto* tmp = front;
			front = tmp->m_Next;
			FreeCommand(*tmp);
		}
	}

	void CCommandHistory::FreeCommand(SCommand& cmd)
	{
		if (&cmd == m_CleanPoint)
		{
			m_CleanPoint = NO_CLEAN_POINT;
		}
		cmd.~SCommand();
	}

	CCommandHistory::SCommand::~SCommand()
	{
		if (m_Command)
			m_Command->~ICommand();
	}

	void CCommandHistory::FreeAll()
	{
		FreeCommandList(m_RedoStack);
		m_RedoStack = nullptr;
		FreeCommandList(m_UndoStack);
		m_UndoStack = nullptr;
		m_Allocator.clear();
		m_DataBuffer.clear();
		m_DataBufferPos = 0;
	}

	void CCommandHistory::OnModified()
	{
		const bool dirty_state = Dirty();
		if (dirty_state == m_LastCommunicatedDirtyState)
		{
			return;
		}
		m_LastCommunicatedDirtyState = dirty_state;
		emit DirtyChanged(dirty_state);
	}
}

#include <moc_CommandHistory.cpp>