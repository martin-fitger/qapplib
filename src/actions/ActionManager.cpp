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

#include <stdexcept>

#include <QtCore/qtimer.h>
#include <QtCore/quuid.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qaction.h>

#include <qapplib/actions/ActionManager.hpp>

namespace qapp
{
	static const auto ACTION_UPDATE_DELAY = std::chrono::milliseconds(100);

	inline CActionManager::action_guid_t CActionManager::ActionGuidFromId(actionid_t id)
	{
		static_assert(sizeof(action_guid_t) == sizeof(QUuid));
		auto g = (id ? QUuid(id) : QUuid());
		return *reinterpret_cast<action_guid_t*>(&g);
	}

	void CActionManager::Init(QObject& app)
	{
		app.installEventFilter(this);
	}

	HAction CActionManager::NewAction(actionid_t action_id, QString title, QString icon_path, const QKeySequence& keys, bool enabled, QAction** out_action)
	{
		auto* action = new QAction(icon_path.isEmpty() ? QIcon() : QIcon(icon_path), title, this);
		action->setShortcut(keys);
		auto tooltip = title;
		tooltip.replace("&", "");
		if (!keys.isEmpty())
			tooltip += " (" + keys.toString() + ")";
		action->setToolTip(tooltip);
		action->setEnabled(enabled);
		if (out_action)
		{
			*out_action = action;
		}
		return RegisterAction(action, action_id);
	}

	HAction CActionManager::RegisterAction(QAction* action, actionid_t action_id)
	{
		#ifdef _DEBUG
			for (size_t i = 0; i < m_Actions.size(); ++i)
			{
				if (m_Actions[i] == action)
				{
					QAPP_ASSERT(false && "CActionManager: Trying to register same action twice");
				}
			}
		#endif

		if (m_Actions.size() >= MAX_ACTION_COUNT)
			throw std::runtime_error("CActionManager: Too many actions. Increase CActionManager::MAX_ACTION_COUNT constant.");

		if (action->isEnabled())
			m_ActionEnabledBits |= (size_t)1 << m_Actions.size();
		else
			m_ActionEnabledBits &= ~((size_t)1 << m_Actions.size());

		const auto action_handle = ActionHandleFromIndex(m_Actions.size());

		QAPP_ASSERT(m_Actions.size() == m_ActionGuids.size());
		m_Actions.push_back(action);
		m_ActionGuids.push_back(ActionGuidFromId(action_id));

		QObject::connect(action, &QAction::triggered, [this, action_handle](bool) { this->OnAction(action_handle); });

		return action_handle;
	}

	QAction* CActionManager::GetAction(HAction handle) const
	{
		return m_Actions[ActionIndexFromHandle(handle)];
	}

	QAction* CActionManager::GetAction(actionid_t action_id) const
	{
		return GetAction(GetActionHandle(action_id));
	}

	HAction CActionManager::GetActionHandle(actionid_t action_id) const
	{
		const auto guid = ActionGuidFromId(action_id);
		for (size_t i = 0; i < m_ActionGuids.size(); ++i)
		{
			if (m_ActionGuids[i] == guid)
			{
				return ActionHandleFromIndex(i);
			}
		}
		return INVALID_ACTION_HANDLE;
	}

	void CActionManager::AddActionTarget(IActionTarget* target)
	{
		m_ActionTargets.push_back(target);
	}

	void CActionManager::RemoveActionTarget(IActionTarget* target)
	{
		auto it = std::find(m_ActionTargets.begin(), m_ActionTargets.end(), target);
		if (m_ActionTargets.end() == it)
			throw std::runtime_error("CActionManager: Trying to remove action target that hasn't been added");
		m_ActionTargets.erase(it);
	}

	void CActionManager::OnUpdateTimer()
	{
		m_UpdateState = EUpdateState_Idle;
		UpdateActions();
	}

	void CActionManager::OnAction(HAction action_handle)
	{
		for (auto& target : m_ActionTargets)
			target->OnAction(action_handle);
	}

	bool CActionManager::eventFilter(QObject *obj, QEvent *event)
	{
		if ((EUpdateState_Idle == m_UpdateState) && IsUpdateEvent(*event))
		{
			EnqueueUpdate();
		}
		return false;
	}

	bool CActionManager::IsUpdateEvent(const QEvent& event) const
	{
		switch (event.type())
		{
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseMove:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::Wheel:
			return true;
		}
		return false;
	}

	void CActionManager::EnqueueUpdate()
	{
		if (EUpdateState_Idle == m_UpdateState)
		{
			m_UpdateState = EUpdateState_Pending;
			QTimer::singleShot(ACTION_UPDATE_DELAY, this, SLOT(OnUpdateTimer()));
		}
	}

	void CActionManager::UpdateActions()
	{
		size_t enabled_bits = 0;
		CActionUpdateContext ctx(enabled_bits);
		for (auto* target : m_ActionTargets)
			target->UpdateActions(ctx);
		const auto change_mask = enabled_bits ^ m_ActionEnabledBits;
		m_ActionEnabledBits = enabled_bits;
		for_each_set_bit(change_mask, [&](int action_index)
			{
				m_Actions[action_index]->setEnabled(0 != (enabled_bits & ((size_t)1 << action_index)));
			});
		
	}
}

#include <moc_ActionManager.cpp>
