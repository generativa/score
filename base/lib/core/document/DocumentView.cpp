// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <QGridLayout>
#include <QLayout>
#include <core/document/Document.hpp>
#include <core/document/DocumentView.hpp>
#include <score/plugins/documentdelegate/DocumentDelegateFactory.hpp>
#include <score/plugins/documentdelegate/DocumentDelegateView.hpp>

#include <score/document/DocumentContext.hpp>
#include <score/widgets/MarginLess.hpp>

namespace score
{
DocumentView::DocumentView(
    DocumentDelegateFactory& fact,
    const Document& doc,
    QObject* parent)
    : QObject{parent}
    , m_document{doc}
    , m_view{fact.makeView(m_document.context().app, this)}
{
}
}
