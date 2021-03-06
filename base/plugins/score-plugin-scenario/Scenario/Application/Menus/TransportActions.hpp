#pragma once

#include <QList>
#include <QPoint>

#include <score/selection/Selection.hpp>

namespace score
{
struct GUIApplicationContext;
struct GUIElements;
}
class QAction;
class QMenu;
class QToolBar;

namespace Scenario
{
class TransportActions : public QObject
{
public:
  TransportActions(const score::GUIApplicationContext&);

  void makeGUIElements(score::GUIElements& ref);

private:
  const score::GUIApplicationContext& m_context;

  QAction* m_play{};
  QAction* m_stop{};
  // QAction* m_goToStart{};
  // QAction* m_goToEnd{};
  QAction* m_stopAndInit{};
  // QAction* m_record{};
};
}
