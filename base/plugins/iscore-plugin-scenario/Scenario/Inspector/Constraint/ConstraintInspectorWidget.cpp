// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ConstraintInspectorWidget.hpp"
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Inspector/MetadataWidget.hpp>
#include <Scenario/Inspector/SelectionButton.hpp>
#include <Scenario/Inspector/Constraint/Widgets/DurationSectionWidget.hpp>
#include <Scenario/Application/ScenarioApplicationPlugin.hpp>
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentPresenter.hpp>
#include <Scenario/Process/ScenarioInterface.hpp>
#include <iscore/document/DocumentContext.hpp>
#include <iscore/widgets/Separator.hpp>
#include <iscore/widgets/TextLabel.hpp>
#include <iscore/widgets/MarginLess.hpp>

namespace Scenario
{
ConstraintInspectorWidget::ConstraintInspectorWidget(
    const Inspector::InspectorWidgetList& widg,
    const ConstraintModel& object,
    std::unique_ptr<ConstraintInspectorDelegate> del,
    const iscore::DocumentContext& ctx,
    QWidget* parent)
    : InspectorWidgetBase{object, ctx, parent}
    , m_widgetList{widg}
    , m_model{object}
    , m_delegate{std::move(del)}
{
  using namespace iscore;
  using namespace iscore::IDocument;
  setObjectName("Constraint");

  ////// HEADER
  // metadata
  m_metadata = new MetadataWidget{m_model.metadata(), ctx.commandStack,
                                  &m_model, this};

  m_metadata->setupConnections(m_model);

  addHeader(m_metadata);

  {
    auto speedWidg = new QWidget{this};
    auto lay = new iscore::MarginLess<QVBoxLayout>{speedWidg};

    // Label
    auto speedLab = new TextLabel{
        "Speed x" + QString::number(m_model.duration.executionSpeed())};
    lay->addWidget(speedLab);

    auto speedLay = new iscore::MarginLess<QGridLayout>;
    lay->addLayout(speedLay);
    speedLay->setHorizontalSpacing(0);
    speedLay->setVerticalSpacing(0);

    auto setSpeedFun = [=](int val) {
      auto& dur = ((ConstraintModel&)(m_model)).duration;
      auto s = double(val) / 100.0;
      if (dur.executionSpeed() != s)
      {
        dur.setExecutionSpeed(s);
      }
    };
    // Buttons
    int btn_col = 0;
    for (int factor : {0, 50, 100, 200, 500})
    {
      auto pb
          = new QPushButton{"x " + QString::number(factor / 100.0), speedWidg};
      pb->setMinimumWidth(35);
      pb->setMaximumWidth(35);
      pb->setContentsMargins(0, 0, 0, 0);
      pb->setStyleSheet(QStringLiteral("QPushButton { margin: 0px; padding: 0px; }"));

      connect(pb, &QPushButton::clicked, this, [=] { setSpeedFun(factor); });
      speedLay->addWidget(pb, 1, btn_col++, 1, 1);
    }

    // Slider
    auto speedSlider = new QSlider{Qt::Horizontal};
    speedSlider->setTickInterval(100);
    speedSlider->setMinimum(-100);
    speedSlider->setMaximum(500);
    speedSlider->setValue(m_model.duration.executionSpeed() * 100.);
    con(m_model.duration, &ConstraintDurations::executionSpeedChanged, this,
        [=](double s) {
          double r = s * 100;
          speedLab->setText("Speed x" + QString::number(s));
          if (r != speedSlider->value())
            speedSlider->setValue(r);
        });

    speedLay->addWidget(speedSlider, 1, btn_col, 1, 1);

    for (int i = 0; i < 5; i++)
      speedLay->setColumnStretch(i, 0);
    speedLay->setColumnStretch(5, 10);
    connect(speedSlider, &QSlider::valueChanged, this, setSpeedFun);

    m_properties.push_back(speedWidg);
  }

  m_delegate->addWidgets_pre(m_properties, this);

  ////// BODY
  auto setAsDisplayedConstraint = new QPushButton{tr("Full view"), this};
  connect(setAsDisplayedConstraint, &QPushButton::clicked, this, [this] {
    auto& base = get<ScenarioDocumentPresenter>(*documentFromObject(m_model));

    base.setDisplayedConstraint(model());
  });

  // Transport
  {
    auto transportWid = new QWidget{this};
    auto transportLay = new iscore::MarginLess<QHBoxLayout>{transportWid};

    auto scenar = dynamic_cast<Scenario::ScenarioInterface*>(m_model.parent());
    ISCORE_ASSERT(scenar);
    transportLay->addStretch(1);

    auto sst = m_model.startState();
    {
      auto btn = SelectionButton::make(
          tr("Start State"), &scenar->state(sst), selectionDispatcher(), this);
      transportLay->addWidget(btn);
    }
    transportLay->addWidget(setAsDisplayedConstraint);

    auto est = m_model.endState();
    {
      auto btn = SelectionButton::make(
          tr("End State"), &scenar->state(est), selectionDispatcher(), this);
      transportLay->addWidget(btn);
    }
    transportLay->addStretch(1);

    m_properties.push_back(transportWid);
  }

  // Separator
  m_properties.push_back(new iscore::HSeparator{this});

  // Durations
  auto& ctrl
      = ctx.app.guiApplicationPlugin<ScenarioApplicationPlugin>();
  m_durationSection
      = new DurationWidget{ctrl.editionSettings(), *m_delegate, this};
  m_properties.push_back(m_durationSection);

  updateDisplayedValues();

  m_delegate->addWidgets_post(m_properties, this);

  // Display data
  updateAreaLayout(m_properties);
}

ConstraintInspectorWidget::~ConstraintInspectorWidget() = default;

ConstraintModel& ConstraintInspectorWidget::model() const
{
  return const_cast<ConstraintModel&>(m_model);
}

QString ConstraintInspectorWidget::tabName()
{
  return tr("Constraint");
}
void ConstraintInspectorWidget::updateDisplayedValues()
{
  m_delegate->updateElements();
}

}
