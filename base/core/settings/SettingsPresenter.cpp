#include <core/settings/SettingsPresenter.hpp>
#include <core/settings/SettingsView.hpp>
#include <core/settings/SettingsModel.hpp>

using namespace iscore;

SettingsPresenter::SettingsPresenter(SettingsModel* model, SettingsView* view):
	QObject{},
	m_model{model},
	m_view{view}
{
	connect(view, &SettingsView::accept,
			this, &SettingsPresenter::on_accept);
	connect(view, &SettingsView::reject,
			this, &SettingsPresenter::on_reject);
}

void SettingsPresenter::addSettingsPresenter(std::unique_ptr<SettingsGroupPresenter>&& presenter)
{
	m_pluginPresenters.insert(std::move(presenter));
}

void SettingsPresenter::on_accept()
{
	for(auto& presenter : m_pluginPresenters)
	{
		presenter->on_accept();
	}
}

void SettingsPresenter::on_reject()
{
	for(auto& presenter : m_pluginPresenters)
	{
		presenter->on_reject();
	}
}
