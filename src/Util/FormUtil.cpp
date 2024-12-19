#include "FormUtil.h"

namespace QuickLoot::Util
{
	RE::FormID FormUtil::ParseFormID(const std::string& identifier)
	{
		std::istringstream ss{ identifier };
		std::string plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);

		RE::FormID localFormID;
		std::istringstream{ id } >> std::hex >> localFormID;

		return RE::TESDataHandler::GetSingleton()->LookupFormID(localFormID, plugin);
	}

	RE::TESForm* FormUtil::GetFormFromIdentifier(const std::string& identifier)
	{
		return RE::TESForm::LookupByID(ParseFormID(identifier));
	}

	std::string FormUtil::GetIdentifierFromForm(RE::TESForm* form)
	{
		if (!form) {
			return "";
		}

		const auto file = form->GetFile();
		const auto plugin = file ? file->fileName : "";

		const RE::FormID formID = form->GetFormID();
		const RE::FormID relativeID = formID & 0x00FFFFFF;

		std::ostringstream ss;
		ss << plugin << "|" << std::hex << relativeID;
		return ss.str();
	}

	std::string FormUtil::GetIdentifierFromForm(RE::BGSRefAlias* form)
	{
		if (!form) {
			return "";
		}

		const auto file = form->owningQuest->GetFile();
		const auto plugin = file ? file->fileName : "";

		const RE::FormID formID = form->owningQuest->GetFormID();
		const RE::FormID relativeID = formID & 0x00FFFFFF;

		std::ostringstream ss;
		ss << plugin << "|" << std::hex << relativeID;
		return ss.str();
	}

	std::string FormUtil::GetModName(RE::TESForm* form)
	{
		const auto file = form ? form->GetFile() : nullptr;
		const auto fileName = file ? file->fileName : nullptr;

		return fileName ? std::filesystem::path{ fileName }.stem().string() : "";
	}

	bool FormUtil::FormExists(std::string_view pluginName, const int formId)
	{
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		const auto* form = dataHandler->LookupForm(formId, pluginName);

		return form;
	}
}
