#include "ScenarioCommandManager.hpp"

#include "source/Process/ScenarioModel.hpp"
#include "source/Process/Temporal/TemporalScenarioViewModel.hpp"
#include "source/Process/Temporal/TemporalScenarioView.hpp"
#include "TemporalScenarioPresenter.hpp"

#include <core/document/Document.hpp>
#include <core/document/DocumentPresenter.hpp>

#include "Document/Event/EventModel.hpp"
#include "Document/Event/EventView.hpp"
#include "Document/Event/EventPresenter.hpp"
#include "Document/Event/EventData.hpp"

#include "Document/TimeNode/TimeNodeModel.hpp"
#include "Document/TimeNode/TimeNodeView.hpp"
#include "Document/TimeNode/TimeNodePresenter.hpp"

#include "Document/Constraint/ViewModels/Temporal/TemporalConstraintView.hpp"
#include "Document/Constraint/ViewModels/Temporal/TemporalConstraintPresenter.hpp"
#include "Document/Constraint/ViewModels/Temporal/TemporalConstraintViewModel.hpp"
#include "Document/Constraint/ConstraintModel.hpp"
#include "Document/Constraint/ConstraintData.hpp"

#include "Commands/Scenario/CreateEvent.hpp"
#include "Commands/Scenario/CreateEventAfterEvent.hpp"
#include "Commands/Scenario/CreateEventAfterEventOnTimeNode.hpp"
#include "Commands/Scenario/CreateConstraint.hpp"
#include "Commands/Scenario/RemoveConstraint.hpp"
#include "Commands/Scenario/RemoveEvent.hpp"
#include "Commands/Scenario/MoveEvent.hpp"
#include "Commands/Scenario/MoveTimeNode.hpp"
#include "Commands/Scenario/MoveConstraint.hpp"
#include "Commands/Scenario/ClearConstraint.hpp"
#include "Commands/Scenario/ClearEvent.hpp"
#include "Commands/RemoveMultipleElements.hpp"
#include "Commands/TimeNode/MergeTimeNodes.hpp"

#include <public_interface/document/DocumentInterface.hpp>
#include <core/presenter/command/OngoingCommandManager.hpp>

#include <algorithm>
#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QApplication>





using namespace Scenario::Command;

template<typename InputVector, typename OutputVector>
void copyIfSelected(const InputVector& in, OutputVector& out)
{
    std::copy_if(begin(in),
                 end(in),
                 back_inserter(out),
                 [](typename InputVector::value_type c)
    {
        return c->isSelected();
    });
}


using namespace iscore;
ScenarioCommandManager::ScenarioCommandManager(TemporalScenarioPresenter* presenter) :
    QObject{presenter},
    m_presenter{presenter},
    m_creationCommandDispatcher{new OngoingCommandDispatcher<MergeStrategy::Undo>{this}},
    m_moveCommandDispatcher{new OngoingCommandDispatcher<MergeStrategy::Simple, CommitStrategy::Redo>{this}},
    m_instantCommandDispatcher{new CommandDispatcher<SendStrategy::Simple>{this}}
{

    // TODO make it more generic (maybe with a QAction ?)
    connect(presenter->m_view, &TemporalScenarioView::deletePressed,
            this, &ScenarioCommandManager::deleteSelection);

    connect(presenter->m_view, &TemporalScenarioView::clearPressed,
            this, &ScenarioCommandManager::clearContentFromSelection);

    connect(presenter->m_view, &TemporalScenarioView::scenarioReleased,
            this, &ScenarioCommandManager::on_scenarioReleased);
}

void ScenarioCommandManager::setupEventPresenter(EventPresenter* e)
{
    connect(e,    &EventPresenter::eventMoved,
            [this] (const EventData& ev) { m_lastData = ev; moveEventAndConstraint(ev); });

    connect(e,    &EventPresenter::eventMovedWithControl,
            [this] (const EventData& ev) { m_lastData = ev; createConstraint(ev); });

    auto commit_fn = [this] ()
    {
        if(m_creationCommandDispatcher->ongoing())
            m_creationCommandDispatcher->commit();

        if(m_moveCommandDispatcher->ongoing())
            m_moveCommandDispatcher->commit();
    };
    connect(e,    &EventPresenter::eventReleased,
            commit_fn);

    connect(e,    &EventPresenter::eventReleasedWithControl,
            commit_fn);

    // TODO manage ctrl being pressed / released globally.
    connect(e,    &EventPresenter::ctrlStateChanged,
            this, &ScenarioCommandManager::on_ctrlStateChanged);

}

void ScenarioCommandManager::setupTimeNodePresenter(TimeNodePresenter* t)
{
    connect(t,    &TimeNodePresenter::timeNodeMoved,
            this, &ScenarioCommandManager::moveTimeNode);

    connect(t,                &TimeNodePresenter::timeNodeReleased,
            m_moveCommandDispatcher, &OngoingCommandDispatcher<>::commit);
}

void ScenarioCommandManager::setupConstraintPresenter(TemporalConstraintPresenter* c)
{
    connect(c,	  &TemporalConstraintPresenter::constraintMoved,
            this, &ScenarioCommandManager::moveConstraint);
    connect(c,                       &TemporalConstraintPresenter::constraintReleased,
            m_moveCommandDispatcher, &OngoingCommandDispatcher<>::commit);
}



// Three cases :
// We are between :
//  an event and nothing -> CreateEventAfterEvent
//  an event and a timenode -> CreateEventAfterEventOnTimeNode
//  an event and another event -> CreateConstraint
void ScenarioCommandManager::createConstraint(EventData data)
{
    using namespace std;
    data.dDate.setMSecs(data.x * m_presenter->m_zoomRatio - model(m_presenter->m_viewModel)->event(data.eventClickedId)->date().msec());
    data.relativeY = data.y / m_presenter->m_view->boundingRect().height();

    auto cmdPath = iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel());

    // We rollback so that we don't get polluted
    // by the "fake" created events / timenodes.
    if(m_moveCommandDispatcher->ongoing())
    {
        m_moveCommandDispatcher->rollback();
    }
    if(m_creationCommandDispatcher->ongoing())
    {
        m_creationCommandDispatcher->rollback();
    }

    QList<EventPresenter*> collidingEvents;
    copy_if(begin(m_presenter->m_events), end(m_presenter->m_events), back_inserter(collidingEvents),
            [](EventPresenter * ev)
    {
        return ev->view()->isUnderMouse();
    });

    QList<TimeNodePresenter*> collidingTimeNodes;
    copy_if(begin(m_presenter->m_timeNodes), end(m_presenter->m_timeNodes), back_inserter(collidingTimeNodes),
            [](TimeNodePresenter * tn)
    {
        return tn->view()->isUnderMouse();
    });

    if(collidingEvents.empty())
    {
        if(collidingTimeNodes.empty())
        {
            emit m_creationCommandDispatcher->submitCommand(new CreateEventAfterEvent(move(cmdPath), data));
        }
        else
        {
            auto tn = collidingTimeNodes.first();
            data.endTimeNodeId = tn->id();
            data.dDate = tn->model()->date() - model(m_presenter->m_viewModel)->event(data.eventClickedId)->date();

            emit m_creationCommandDispatcher->submitCommand(new CreateEventAfterEventOnTimeNode(move(cmdPath), data));
        }
    }
    else
    {
        emit m_creationCommandDispatcher->submitCommand(new CreateConstraint(move(cmdPath),
                                                data.eventClickedId,
                                                collidingEvents.first()->id()));
    }
}

// TODO on_scenarioMoved instead?
void ScenarioCommandManager::on_scenarioReleased(QPointF point, QPointF scenePoint)
{
    EventData data {};
    data.eventClickedId = m_presenter->m_events.back()->id();
    data.x = point.x();
    data.dDate.setMSecs(point.x() * m_presenter->m_zoomRatio);
    data.y = point.y();
    data.relativeY = point.y() /  m_presenter->m_view->boundingRect().height();
    data.scenePos = scenePoint;

    TimeNodeView* tnv =  dynamic_cast<TimeNodeView*>(m_presenter->m_view->scene()->itemAt(scenePoint, QTransform()));

    if(tnv)
    {
        for(auto timeNode : m_presenter->m_timeNodes)
        {
            if(timeNode->view() == tnv)
            {
                data.endTimeNodeId = timeNode->id();
                data.dDate = timeNode->model()->date();
                data.x = data.dDate.toPixels(m_presenter->m_zoomRatio);
                break;
            }
        }
    }

    auto cmd = new CreateEvent{
                    iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                    data};

    emit m_creationCommandDispatcher->submitCommand(cmd);
    emit m_creationCommandDispatcher->commit();
}

void ScenarioCommandManager::clearContentFromSelection()
{
    // 1. Select items
    std::vector<TemporalConstraintPresenter*> constraintsToRemove;
    std::vector<EventPresenter*> eventsToRemove;

    copyIfSelected(m_presenter->m_constraints, constraintsToRemove);
    copyIfSelected(m_presenter->m_events, eventsToRemove);

    QVector<iscore::SerializableCommand*> commands;

    // 3. Create a Delete command for each. For now : only emptying.
    for(auto& constraint : constraintsToRemove)
    {
        commands.push_back(
            new ClearConstraint(
                iscore::IDocument::path(viewModel(constraint)->model())));
    }

    for(auto& event : eventsToRemove)
    {
        commands.push_back(
            new ClearEvent(
                iscore::IDocument::path(event->model())));
    }

    // 4. Make a meta-command that binds them all and calls undo & redo on the queue.
    auto cmd = new RemoveMultipleElements {std::move(commands) };
    emit m_creationCommandDispatcher->submitCommand(cmd);
    emit m_creationCommandDispatcher->commit();
}

void ScenarioCommandManager::deleteSelection()
{
    // TODO quelques comportements bizarres à régler ...

    //*
    // 1. Select items
    std::vector<TemporalConstraintPresenter*> constraintsToRemove;
    std::vector<EventPresenter*> eventsToRemove;

    copyIfSelected(m_presenter->m_constraints, constraintsToRemove);
    copyIfSelected(m_presenter->m_events, eventsToRemove);

    if(constraintsToRemove.size() != 0 || eventsToRemove.size() != 0)
    {
        QVector<iscore::SerializableCommand*> commands;

        // 2. Create a Delete command for each. For now : only emptying.
        for(auto& constraint : constraintsToRemove)
        {
            commands.push_back(
                new RemoveConstraint(
                    iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                    constraint->abstractConstraintViewModel()->model()));
        }

        for(auto& event : eventsToRemove)
        {
            commands.push_back(
                new RemoveEvent(
                    iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                    event->model()));
        }

        // 3. Make a meta-command that binds them all and calls undo & redo on the queue.
        auto cmd = new RemoveMultipleElements {std::move(commands) };
        emit m_instantCommandDispatcher->submitCommand(cmd);
    }
}

void ScenarioCommandManager::moveEventAndConstraint(EventData data)
{
    data.dDate.setMSecs(data.x * m_presenter->m_zoomRatio);
    data.relativeY = data.y / m_presenter->m_view->boundingRect().height();
    auto eventTN = findById(m_presenter->m_events, data.eventClickedId)->model()->timeNode();

    if(m_creationCommandDispatcher->ongoing())
    {
        m_creationCommandDispatcher->rollback();
    }

    QList<TimeNodePresenter*> collidingTimeNodes;
    copy_if(begin(m_presenter->m_timeNodes), end(m_presenter->m_timeNodes), std::back_inserter(collidingTimeNodes),
            [=](TimeNodePresenter * tn)
    {
        if (eventTN != tn->id())
            return tn->view()->isUnderMouse();
        return false;
    });

    if (collidingTimeNodes.isEmpty())
    {
        auto cmd = new MoveEvent(iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                                 data);

        emit m_moveCommandDispatcher->submitCommand(cmd);
    }
    else
    {
        m_moveCommandDispatcher->rollback();
        auto cmd = new MergeTimeNodes(iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                                      collidingTimeNodes.first()->id(),
                                      eventTN);

        emit m_instantCommandDispatcher->submitCommand(cmd);
    }

}

void ScenarioCommandManager::moveConstraint(ConstraintData data)
{
    data.dDate.setMSecs(data.x * m_presenter->m_zoomRatio);
    data.relativeY = data.y / m_presenter->m_view->boundingRect().height();

    auto cmd = new MoveConstraint(iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                                  data);

    emit m_moveCommandDispatcher->submitCommand(cmd);
}

void ScenarioCommandManager::moveTimeNode(EventData data)
{
    auto ev = findById(m_presenter->m_events, data.eventClickedId);
    data.y = ev->view()->y();
    data.dDate.setMSecs(data.x * m_presenter->m_zoomRatio);
    data.relativeY = data.y / m_presenter->m_view->boundingRect().height();


    auto cmd = new MoveTimeNode(iscore::IDocument::path(m_presenter->m_viewModel->sharedProcessModel()),
                                data);

    emit m_moveCommandDispatcher->submitCommand(cmd);
}



void ScenarioCommandManager::on_ctrlStateChanged(bool ctrlPressed)
{
    if(!ongoingCommand())
    {
        return;
    }

    if(ctrlPressed)
    {
        createConstraint(m_lastData);
    }
    else
    {
        moveEventAndConstraint(m_lastData);
    }
}

bool ScenarioCommandManager::ongoingCommand()
{
    return m_creationCommandDispatcher->ongoing() || m_moveCommandDispatcher->ongoing();
}
