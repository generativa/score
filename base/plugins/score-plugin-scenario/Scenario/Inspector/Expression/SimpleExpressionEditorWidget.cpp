// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QToolButton>
#include <State/Expression.hpp>
#include <State/Relation.hpp>
#include <score/tools/std/Optional.hpp>

#include "SimpleExpressionEditorWidget.hpp"
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Explorer/Explorer/DeviceExplorerModel.hpp>
#include <Explorer/Widgets/AddressAccessorEditWidget.hpp>
#include <Scenario/Inspector/ExpressionValidator.hpp>
#include <Inspector/InspectorSectionWidget.hpp>

#include <score/widgets/MarginLess.hpp>
#include <score/widgets/SetIcons.hpp>
#include <score/widgets/SignalUtils.hpp>
#include <score/widgets/TextLabel.hpp>

namespace Scenario
{
SimpleExpressionEditorWidget::SimpleExpressionEditorWidget(
    const score::DocumentContext& doc, int index, QWidget* parent, QMenu* menu)
    : QWidget(parent), id{index}
{
  auto mainLay = new score::MarginLess<QHBoxLayout>{this};

  m_binOperator = new QComboBox{this};

  m_address = new Explorer::AddressAccessorEditWidget{
      doc.plugin<Explorer::DeviceDocumentPlugin>().explorer(),
      this};
  m_ok = new TextLabel{QStringLiteral("/!\\ "), this};

  m_comparator = new QComboBox{this};

  m_value = new QLineEdit{this};

  auto btnWidg = new QWidget{this};
  auto btnLay = new score::MarginLess<QHBoxLayout>{btnWidg};
  m_rmBtn = new QToolButton{btnWidg};
  m_rmBtn->setText(QStringLiteral("-"));
  m_rmBtn->setMaximumSize(30, 30);
  auto remIcon = makeIcons(
      ":/icons/condition_remove_on.png", ":/icons/condition_remove_off.png");

  m_rmBtn->setIcon(remIcon);

  m_addBtn = new QToolButton{btnWidg};
  m_addBtn->setText(QStringLiteral("+"));
  m_addBtn->setMaximumSize(30, 30);
  m_addBtn->setVisible(false);
  auto addIcon = makeIcons(
      ":/icons/condition_add_on.png", ":/icons/condition_add_off.png");

  m_addBtn->setIcon(addIcon);

  m_menuBtn = new Inspector::MenuButton{this};
  m_menuBtn->setObjectName(QStringLiteral("SettingsMenu"));
  m_menuBtn->setMaximumSize(30,30);
  m_menuBtn->setMenu(menu);

  QSizePolicy sp = m_menuBtn->sizePolicy();
  sp.setRetainSizeWhenHidden(true);
  m_menuBtn->setSizePolicy(sp);
  m_menuBtn->setVisible(false);

  btnLay->addWidget(m_rmBtn);
  btnLay->addWidget(m_addBtn);
  btnLay->addWidget(m_menuBtn);

  // Main Layout

  mainLay->addWidget(m_ok);
  mainLay->addWidget(m_binOperator, 0, Qt::AlignHCenter);
  mainLay->addWidget(m_address, 10);
  mainLay->addWidget(m_comparator, 0, Qt::AlignHCenter);
  mainLay->addWidget(m_value, 2);

  mainLay->addWidget(btnWidg, 0, Qt::AlignRight);

  // Connections

  connect(m_rmBtn, &QPushButton::clicked, this, [=]() { emit removeTerm(id); });
  connect(m_addBtn, &QPushButton::clicked, this, [=]() { emit addTerm(); });

  /// EDIT FINSHED
  connect(
      m_address, &Explorer::AddressAccessorEditWidget::addressChanged, this,
      [&]() { on_editFinished(); });
  connect(m_comparator, &QComboBox::currentTextChanged, this, [&] {
    on_editFinished();
  });
  connect(
      m_value, &QLineEdit::editingFinished, this,
      &SimpleExpressionEditorWidget::on_editFinished);
  connect(m_binOperator, &QComboBox::currentTextChanged, this, [&] {
    on_editFinished();
  });

  // enable value field
  connect(
      m_comparator, SignalUtils::QComboBox_currentIndexChanged_int(), this,
      &SimpleExpressionEditorWidget::on_comparatorChanged);

  m_ok->setVisible(false);
  m_value->setEnabled(false);

  // Fill ComboBox

  m_binOperator->setObjectName("BinOpComboBox");
  m_comparator->setObjectName("BinOpComboBox");

  m_binOperator->addItem(" ");
  m_binOperator->addItem("&");
  m_binOperator->addItem("|");

  auto& lst = ExpressionEditorComparators();

  for (auto& c : lst)
  {
    m_comparator->addItem(c.second, QVariant::fromValue(c.first));
  }

  m_comparator->setCurrentText(
      lst.at(ExpressionEditorComparator::AlwaysTrue));

  QSizePolicy sp_retain = m_binOperator->sizePolicy();
  sp_retain.setRetainSizeWhenHidden(true);
  m_binOperator->setSizePolicy(sp_retain);

  if (id == 0)
    m_binOperator->setVisible(false);
  else
    m_binOperator->setCurrentIndex(1);
}

State::Expression SimpleExpressionEditorWidget::relation()
{
  int i = 1;
  QString expr = currentRelation();

  m_validator.validate(expr, i);
  if (m_validator.validate(expr, i) == QValidator::State::Acceptable)
  {
    return *m_validator.get();
  }

  else
    return State::Expression{};
}

optional<State::BinaryOperator> SimpleExpressionEditorWidget::binOperator()
{
  switch (m_binOperator->currentIndex())
  {
    case 1:
      return State::BinaryOperator::AND;
    case 2:
      return State::BinaryOperator::OR;
    default:
      return ossia::none;
  }
}

void SimpleExpressionEditorWidget::setRelation(State::Relation r)
{
  auto lptr = r.lhs.target<ossia::value>();
  auto rptr = r.rhs.target<ossia::value>();
  if (lptr && rptr)
  {
    auto lv = *lptr;
    auto rv = *rptr;

    if (r.op == ossia::expressions::comparator::EQUAL && lv == rv)
    {
      m_comparator->setCurrentIndex(ExpressionEditorComparator::AlwaysTrue);
    }
    else
    {
      m_comparator->setCurrentIndex(ExpressionEditorComparator::AlwaysFalse);
    }
    m_address->setAddress(State::AddressAccessor{});
    m_value->clear();
  }
  else
  {
    if (auto addr_ptr = r.lhs.target<State::Address>())
    {
      m_address->setAddress(State::AddressAccessor{*addr_ptr});
    }
    else if (auto acc_ptr = r.lhs.target<State::AddressAccessor>())
    {
      m_address->setAddress(*acc_ptr);
    }

    auto s = State::toString(r.rhs);
    m_value->setText(s);

    m_comparator->setCurrentIndex(static_cast<int>(r.op));

    m_relation = State::toString(r);

    int i;
    m_ok->setVisible(
        m_validator.validate(m_relation, i) != QValidator::State::Acceptable);
  }
}

void SimpleExpressionEditorWidget::setPulse(State::Pulse p)
{
  m_address->setAddress(State::AddressAccessor{p.address});
  m_value->clear();

  m_comparator->setCurrentIndex(ExpressionEditorComparator::Pulse);
  m_relation = State::toString(p);

  int i;
  m_ok->setVisible(
      m_validator.validate(m_relation, i) != QValidator::State::Acceptable);
}

void SimpleExpressionEditorWidget::setOperator(State::BinaryOperator o)
{
  switch (o)
  {
    case State::BinaryOperator::AND:
      m_binOperator->setCurrentIndex(1);
      m_op = "and";
      break;
    case State::BinaryOperator::OR:
      m_binOperator->setCurrentIndex(2);
      m_op = "or";
      break;
    default:
      m_binOperator->setCurrentIndex(0);
      m_op = "";
      break;
  }
}

void SimpleExpressionEditorWidget::setOperator(State::UnaryOperator u)
{
  // TODO : add the unary operator
  /*    switch (u) {
          case State::UnaryOperator::Not :
              break;
          default:
              m_binOperator->setCurrentIndex(0);
              break;
      }
      */
}

void SimpleExpressionEditorWidget::on_editFinished()
{
  QString expr = currentRelation();
  if (expr == m_relation && m_op == currentOperator())
    return;

  int i = 1;
  m_op = currentOperator();
  m_relation = expr;
  bool b = m_validator.validate(expr, i) == QValidator::State::Acceptable;
  m_ok->setVisible(!b);
  if (b)
    emit editingFinished();
}

void SimpleExpressionEditorWidget::on_comparatorChanged(int i)
{
  switch (i)
  {
    case ExpressionEditorComparator::Equal:
    case ExpressionEditorComparator::Different:
    case ExpressionEditorComparator::Greater:
    case ExpressionEditorComparator::Lower:
    case ExpressionEditorComparator::GreaterEqual:
    case ExpressionEditorComparator::LowerEqual:
      m_address->setEnabled(true);
      m_value->setEnabled(true);
      m_address->setVisible(true);
      m_value->setVisible(true);
      break;

    case ExpressionEditorComparator::Pulse:
      m_address->setEnabled(true);
      m_value->setEnabled(false);
      m_address->setVisible(true);
      m_value->setVisible(false);
      break;

    case ExpressionEditorComparator::AlwaysTrue:
    case ExpressionEditorComparator::AlwaysFalse:
      m_address->setEnabled(false);
      m_value->setEnabled(false);
      m_address->setVisible(false);
      m_value->setVisible(false);
      break;

    default:
      m_address->setEnabled(false);
      m_value->setEnabled(false);
      m_address->setVisible(false);
      m_value->setVisible(false);
      break;
  }
}

QString SimpleExpressionEditorWidget::currentRelation()
{
  QString addr = m_address->addressString();

  switch (m_comparator->currentIndex())
  {
    case ExpressionEditorComparator::Greater:
    case ExpressionEditorComparator::Lower:
    {
      QString expr = m_address->addressString();
      expr += " ";
      expr += m_comparator->currentText();
      expr += " ";
      expr += m_value->text();
      return expr;
    }
    case ExpressionEditorComparator::Equal:
      return addr + " == " + m_value->text();
    case ExpressionEditorComparator::GreaterEqual:
      return addr + " >= " + m_value->text();
    case ExpressionEditorComparator::LowerEqual:
      return addr + " <= " + m_value->text();
    case ExpressionEditorComparator::Different:
      return addr + " != " + m_value->text();

    case ExpressionEditorComparator::Pulse:
    {
      QString expr = m_address->addressString() + " impulse";
      return expr;
    }
    case ExpressionEditorComparator::AlwaysTrue:
    {
      return State::defaultTrueExpression().toString();
    }
    case ExpressionEditorComparator::AlwaysFalse:
    {
      return State::defaultFalseExpression().toString();
    }
  }

  return "";
}

QString SimpleExpressionEditorWidget::currentOperator()
{
  return m_binOperator->currentText();
}

void SimpleExpressionEditorWidget::enableRemoveButton(bool b)
{
  m_rmBtn->setVisible(b);
}

void SimpleExpressionEditorWidget::enableAddButton(bool b)
{
  m_addBtn->setVisible(b);

  QSizePolicy sp = m_menuBtn->sizePolicy();
  sp.setRetainSizeWhenHidden(!b);
  m_menuBtn->setSizePolicy(sp);
}

void SimpleExpressionEditorWidget::enableMenuButton(bool b)
{
  m_menuBtn->setVisible(b);
}

const std::map<ExpressionEditorComparator, QString>&
ExpressionEditorComparators()
{
  static const std::map<ExpressionEditorComparator, QString> map{
      {ExpressionEditorComparator::Equal, "="},
      {ExpressionEditorComparator::Different, QString::fromUtf8("≠")},
      {ExpressionEditorComparator::Greater, ">"},
      {ExpressionEditorComparator::Lower, "<"},
      {ExpressionEditorComparator::GreaterEqual, QString::fromUtf8("≥")},
      {ExpressionEditorComparator::LowerEqual, QString::fromUtf8("≤")},
      {ExpressionEditorComparator::Pulse, "Pulse"},
      {ExpressionEditorComparator::AlwaysTrue, "True"},
      {ExpressionEditorComparator::AlwaysFalse, "False"}};

  return map;
}
}
