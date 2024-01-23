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

#include <cstdint>
#include <vector>

#include <QtCore/qobject.h>
#include <QtGui/qkeysequence.h>

#include <qapplib/Debug.h>
#include <qapplib/actions/ActionTarget.h>
#include <qapplib/utils/Bits.h>

class QAction;

namespace qapp
{
	typedef const char* actionid_t;
	static const std::intptr_t NO_ACTIONID = 0;

	enum class EActionTargetPrio
	{
		Workbench,
		Editor,
		View,
	};

	class CActionManager : public QObject
	{
		Q_OBJECT
	public:
		void Init(QObject& app);

		HAction NewAction(actionid_t action_id, QString title, QString icon_path, const QKeySequence& keys = QKeySequence(), bool enabled = false, QAction** out_action = nullptr);

		HAction RegisterAction(QAction* action, actionid_t action_id = (actionid_t)NO_ACTIONID);

		QAction* GetAction(HAction handle) const;
		QAction* GetAction(actionid_t action_id) const;

		HAction GetActionHandle(actionid_t action_id) const;

		void AddActionTarget(IActionTarget* target, EActionTargetPrio prio);

		void RemoveActionTarget(IActionTarget* target);

		void UpdateActions();

	private Q_SLOTS:
		void OnUpdateTimer();

	private:
		static const uint32_t MAX_ACTION_COUNT = 64;

		typedef std::pair<uint64_t, uint64_t> action_guid_t;

		void OnAction(HAction action_handle);

		// QObject overrides
		bool eventFilter(QObject *obj, QEvent *event) override;

		bool IsUpdateEvent(const QEvent& event) const;

		void EnqueueUpdate();

		inline static uint32_t ActionIndexFromHandle(HAction handle) { QAPP_ASSERT(handle);  return handle - 1; }
		inline static HAction  ActionHandleFromIndex(size_t index) { return (HAction)index + 1; }

		inline static action_guid_t ActionGuidFromId(actionid_t id);

		enum EUpdateState
		{
			EUpdateState_Idle,
			EUpdateState_Pending,
		};

		EUpdateState m_UpdateState = EUpdateState_Idle;

		size_t m_ActionEnabledBits = 0;
		std::vector<QAction*> m_Actions;
		std::vector<std::pair<IActionTarget*, EActionTargetPrio>> m_ActionTargets;
		
		std::vector<action_guid_t> m_ActionGuids;
	};
}