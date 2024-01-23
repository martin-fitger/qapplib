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

#pragma once

#include <QtCore/qobject.h>

#include <memory>
#include <optional>

#include <qapplib/utils/PageBufferStream.h>
#include <qapplib/utils/PageStackAllocator.h>
#include "Command.h"

namespace qapp
{
	class IEditor;

	class CCommandHistory: public QObject
	{
		Q_OBJECT
	public:
		CCommandHistory(IEditor& editor, CPagePool& page_pool);
		~CCommandHistory();

		void Clear();

		template<class TCommand, typename... TArgs>
		inline void NewCommand(TArgs&&... args);
	
		template <typename TLambda>
		inline bool NewCommandOptional(TLambda&& lambda);

		bool CanUndo() const;
		
		bool CanRedo() const;
		
		void Undo();
		
		void Redo();

		void ClearRedoStack();

		void SetCleanAtCurrentPosition();

		inline bool Dirty() const { return m_CleanPoint != m_UndoStack; }

	Q_SIGNALS:
		void DirtyChanged(bool dirty);

	private:
		struct SCommand
		{
			~SCommand();

			ICommand* m_Command = nullptr;
			SCommand* m_Next = nullptr;
			page_stack_allocator::state_t m_AllocatorState = 0;
			size_t    m_DataSize = 0;
			bool      m_Incomplete = false;
		};

		void Do(SCommand& cmd);

		void Undo(SCommand& cmd);

		void FreeCommandList(SCommand* front);

		void FreeCommand(SCommand& cmd);

		void FreeAll();

		void OnModified();

		IEditor&    m_Editor;
		page_buffer m_DataBuffer;
		page_stack_allocator m_Allocator;
		size_t      m_DataBufferPos = 0;
		SCommand*   m_UndoStack = nullptr;
		SCommand*   m_RedoStack = nullptr;
		const SCommand* m_CleanPoint = nullptr;
		bool m_LastCommunicatedDirtyState = false;

		static const SCommand* NO_CLEAN_POINT;
	};

	template<class TCommand, typename... TArgs>
	inline void CCommandHistory::NewCommand(TArgs&&... args)
	{
		NewCommandOptional([&](SCommandCreationContext& ctx) -> auto
		{
			return std::make_optional<TCommand>(ctx, std::forward<TArgs>(args)...);
		});
	}

	template <typename TLambda>
	inline bool CCommandHistory::NewCommandOptional(TLambda&& lambda)
	{
		ClearRedoStack();
		const auto allocator_state = m_Allocator.state();
		page_buffer_ostream data(m_DataBuffer, m_DataBufferPos);
		SCommandCreationContext ctx_create(m_Editor, data);
		auto cmd_optional = lambda(ctx_create);
		if (!cmd_optional.has_value())
		{
			m_DataBuffer.resize(m_DataBufferPos);
			return false;
		}
		typedef typename decltype(cmd_optional)::value_type command_type;
		auto* cmd = m_Allocator.New<SCommand>();
		cmd->m_Command = m_Allocator.New<command_type>(std::move(cmd_optional.value()));
		cmd->m_AllocatorState = allocator_state;
		cmd->m_DataSize = m_DataBuffer.size() - m_DataBufferPos;
		cmd->m_Incomplete = ctx_create.m_Incomplete;
		Do(*cmd);
		cmd->m_Next = m_UndoStack;
		m_UndoStack = cmd;
		OnModified();
		return true;
	}
}
