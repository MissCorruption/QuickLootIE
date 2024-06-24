#include "FormUtil.hpp"

auto FormUtil::GetFormFromIdentifier(const std::string& a_identifier) -> RE::TESForm*
{
	std::istringstream ss{ a_identifier };
	std::string plugin, id;

	std::getline(ss, plugin, '|');
	std::getline(ss, id);
	RE::FormID relativeID;
	std::istringstream{ id } >> std::hex >> relativeID;
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	return dataHandler ? dataHandler->LookupForm(relativeID, plugin) : nullptr;
}

auto FormUtil::GetIdentifierFromForm(RE::TESForm* a_form) -> std::string
{
	if (!a_form) {
		return ""s;
	}

	auto file = a_form->GetFile();
	auto plugin = file ? file->fileName : "";

	RE::FormID formID = a_form->GetFormID();
	RE::FormID relativeID = formID & 0x00FFFFFF;

	std::ostringstream ss;
	ss << plugin << "|" << std::hex << relativeID;
	return ss.str();
}

auto FormUtil::GetIdentifierFromForm(RE::BGSRefAlias* a_form) -> std::string
{
	if (!a_form) {
		return ""s;
	}

	auto file = a_form->owningQuest->GetFile();
	auto plugin = file ? file->fileName : "";

	RE::FormID formID = a_form->owningQuest->GetFormID();
	RE::FormID relativeID = formID & 0x00FFFFFF;

	std::ostringstream ss;
	ss << plugin << "|" << std::hex << relativeID;
	return ss.str();
}

auto FormUtil::GetModName(RE::TESForm* a_form) -> std::string
{
	auto file = a_form ? a_form->GetFile() : nullptr;
	auto fileName = file ? file->fileName : nullptr;

	return fileName ? std::filesystem::path{ fileName }.stem().string() : ""s;
}
