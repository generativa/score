#include "CoreApplicationPlugin.hpp"
#include <iscore_git_info.hpp>

#include <core/settings/Settings.hpp>
#include <core/settings/SettingsView.hpp>
#include <core/view/View.hpp>
#include <iscore/plugins/documentdelegate/DocumentDelegateFactoryInterface.hpp>
#include <iscore/menu/MenuInterface.hpp>
#include <QMessageBox>
#include <QRecentFilesMenu.h>
#include <QMessageAuthenticationCode>
namespace iscore
{

CoreApplicationPlugin::CoreApplicationPlugin(const ApplicationContext& app, Presenter& pres):
    GUIApplicationContextPlugin{app},
    m_presenter{pres}
{

}

void CoreApplicationPlugin::newDocument()
{
    m_presenter.m_docManager.newDocument(
                context,
                getStrongId(m_presenter.m_docManager.documents()),
                *context.components.factory<iscore::DocumentDelegateList>().begin());

}

void CoreApplicationPlugin::load()
{
    m_presenter.m_docManager.loadFile(context);
}

void CoreApplicationPlugin::save()
{
    m_presenter.m_docManager.saveDocument(*m_presenter.m_docManager.currentDocument());
}

void CoreApplicationPlugin::saveAs()
{
    m_presenter.m_docManager.saveDocumentAs(*m_presenter.m_docManager.currentDocument());
}

void CoreApplicationPlugin::close()
{
    if(auto doc = m_presenter.m_docManager.currentDocument())
        m_presenter.m_docManager.closeDocument(context, *doc);
}

void CoreApplicationPlugin::quit()
{
    m_presenter.m_view->close();
}

void CoreApplicationPlugin::openSettings()
{
    m_presenter.m_settings.view().exec();
}

void CoreApplicationPlugin::about()
{
    auto version_text =
            QString("%1.%2.%3-%4 '%5'\n\n")
            .arg(ISCORE_VERSION_MAJOR)
            .arg(ISCORE_VERSION_MINOR)
            .arg(ISCORE_VERSION_PATCH)
            .arg(ISCORE_VERSION_EXTRA)
            .arg(ISCORE_CODENAME);

    QString commit{GIT_COMMIT};
    if(!commit.isEmpty())
    {
        version_text += tr("Commit: \n%1\n").arg(commit);
    }

    QMessageBox::about(nullptr,
                       tr("About i-score"),
                       tr("With love and sweat from the i-score team. \nVersion:\n")
                       + version_text);
}

void CoreApplicationPlugin::loadStack()
{
    m_presenter.m_docManager.loadStack(context);
}

void CoreApplicationPlugin::saveStack()
{
    m_presenter.m_docManager.saveStack();
}

GUIElements CoreApplicationPlugin::makeGUIElements()
{
    std::vector<Menu> menus;
    menus.reserve(10);
    auto file = new QMenu{tr("&File")};
    auto edit = new QMenu{tr("&Edit")};
    auto object = new QMenu{tr("&Object")};
    auto play = new QMenu{tr("&Play")};
    auto tool = new QMenu{tr("&Tool")};
    auto view = new QMenu{tr("&View")};
    auto settings = new QMenu{tr("&Settings")};
    auto about = new QMenu{tr("&About")};
    menus.emplace_back(file, Menus::File(), Menu::is_toplevel{}, 0);
    menus.emplace_back(edit, Menus::Edit(), Menu::is_toplevel{}, 1);
    menus.emplace_back(object, Menus::Object(), Menu::is_toplevel{}, 2);
    menus.emplace_back(play, Menus::Play(), Menu::is_toplevel{}, 3);
    menus.emplace_back(tool, Menus::Tool(), Menu::is_toplevel{}, 4);
    menus.emplace_back(view, Menus::View(), Menu::is_toplevel{}, 5);
    menus.emplace_back(settings, Menus::Settings(), Menu::is_toplevel{}, 6);

    // Menus are by default at int_max - 1 so that they will be sorted before
    menus.emplace_back(about, Menus::About(), Menu::is_toplevel{}, std::numeric_limits<int>::max());

    auto export_menu = new QMenu{tr("&Export")};
    menus.emplace_back(export_menu, Menus::Export());

    auto windows_menu = new QMenu{tr("&Windows")};
    menus.emplace_back(windows_menu, Menus::Windows());

    std::vector<Action> actions;
    ////// File //////
    // New
    // ---
    // Load
    // Recent
    // Save
    // Save As
    // ---
    // Export
    // ---
    // Close
    // Quit

    {
        auto new_doc = new QAction(tr("&New"), m_presenter.view());
        connect(new_doc, &QAction::triggered, this, &CoreApplicationPlugin::newDocument);
        file->addAction(new_doc);
        actions.emplace_back(new_doc, "New", "Common", Action::EnablementContext::Application, QKeySequence::New);
    }

    file->addSeparator();


    {
        auto load_doc = new QAction(tr("&Load"), m_presenter.view());
        connect(load_doc, &QAction::triggered, this, &CoreApplicationPlugin::load);
        actions.emplace_back(load_doc, "Load", "Common", Action::EnablementContext::Application, QKeySequence::Open);
        file->addAction(load_doc);
    }

    file->addMenu(m_presenter.m_docManager.recentFiles());

    {
        auto save_doc = new QAction(tr("&Save"), m_presenter.view());
        connect(save_doc, &QAction::triggered, this, &CoreApplicationPlugin::save);
        actions.emplace_back(save_doc, "Save", "Common", Action::EnablementContext::Document, QKeySequence::Save);
        file->addAction(save_doc);
    }

    {
        auto saveas_doc = new QAction(tr("Save &As..."), m_presenter.view());
        connect(saveas_doc, &QAction::triggered, this, &CoreApplicationPlugin::saveAs);
        actions.emplace_back(saveas_doc, "SaveAs", "Common", Action::EnablementContext::Document, QKeySequence::SaveAs);
        file->addAction(saveas_doc);
    }

    file->addSeparator();

    file->addMenu(export_menu);
#ifdef ISCORE_DEBUG
    // Add command stack import / export
    {
        auto loadStack_act = new QAction(tr("&Load a stack"), m_presenter.view());
        connect(loadStack_act, &QAction::triggered, this, &CoreApplicationPlugin::loadStack);
        actions.emplace_back(loadStack_act, "LoadStack", "Common", Action::EnablementContext::Document, QKeySequence::UnknownKey);
        export_menu->addAction(loadStack_act);
    }

    {
        auto saveStack_act = new QAction(tr("&Save a stack"), m_presenter.view());
        connect(saveStack_act, &QAction::triggered, this, &CoreApplicationPlugin::saveStack);
        actions.emplace_back(saveStack_act, "SaveStack", "Common", Action::EnablementContext::Document, QKeySequence::UnknownKey);
        export_menu->addAction(saveStack_act);
    }
#endif

    file->addSeparator();

    {
        auto close_act = new QAction(tr("&Close"), m_presenter.view());
        connect(close_act, &QAction::triggered, this, &CoreApplicationPlugin::close);
        actions.emplace_back(close_act, "Close", "Common", Action::EnablementContext::Document, QKeySequence::Close);
        file->addAction(close_act);
    }

    {
        auto quit_act = new QAction(tr("&Quit"), m_presenter.view());
        connect(quit_act, &QAction::triggered, this, &CoreApplicationPlugin::quit);
        actions.emplace_back(quit_act, "Quit", "Common", Action::EnablementContext::Application, QKeySequence::Quit);
        file->addAction(quit_act);
    }


    ////// View //////
    view->addMenu(windows_menu);

    ////// Settings //////
    {
        auto settings_act = new QAction(tr("&Settings"), m_presenter.view());
        connect(settings_act, &QAction::triggered, this, &CoreApplicationPlugin::openSettings);
        actions.emplace_back(settings_act, "OpenSettings", "Common", Action::EnablementContext::Application, QKeySequence::Preferences);
        settings->addAction(settings_act);
    }

    ////// About /////
    {
        auto about_act = new QAction(tr("&About"), m_presenter.view());
        connect(about_act, &QAction::triggered, this, &CoreApplicationPlugin::about);
        actions.emplace_back(about_act, "About", "Common", Action::EnablementContext::Application, QKeySequence::UnknownKey);
        about->addAction(about_act);
    }


    return std::make_tuple(menus, std::vector<Toolbar>{}, actions);
}

}