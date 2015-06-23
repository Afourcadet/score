#pragma once

#include "MoveEvent.hpp"


class EventModel;
class ConstraintModel;

namespace Scenario
{
    namespace Command
    {

        class MoveNewEvent : public iscore::SerializableCommand
        {
                ISCORE_COMMAND_DECL("MoveNewEvent", "MoveNewEvent")
            public:

                MoveNewEvent(ObjectPath&& scenarioPath,
                            id_type<ConstraintModel> constraintId,
                            id_type<EventModel> eventId,
                            const TimeValue& date,
                            const double y,
                            bool yLocked);

              virtual void undo() override;
              virtual void redo() override;

              void update(const ObjectPath& path,
                      const id_type<ConstraintModel> ,
                      const id_type<EventModel>& id,
                      const TimeValue& date,
                      const double y,
                      bool yLocked)
              {
                  m_cmd->update(path, id, date, ExpandMode::Fixed);
                  m_y = y;
                  m_yLocked = yLocked;
              }

              const ObjectPath& path() const
              { return m_path; }

            protected:
              virtual void serializeImpl(QDataStream&) const override;
              virtual void deserializeImpl(QDataStream&) override;

            private:
              ObjectPath m_path;
              id_type<ConstraintModel> m_constraintId{};

              MoveEvent* m_cmd;
              double m_y;
              bool m_yLocked;
        };
    }
}
