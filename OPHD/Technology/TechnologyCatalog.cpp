#include "TechnologyCatalog.h"
#include "../XmlSerializer.h"

#include <NAS2D/Utility.h>
#include <NAS2D/Filesystem.h>
#include <NAS2D/ParserHelper.h>

#include <algorithm>
#include <stdexcept>


using namespace NAS2D;

namespace
{
	std::map<std::string, Technology::Modifier::Modifies> StringToModifier =
	{
		{"agriculture", Technology::Modifier::Modifies::AgricultureEfficiency},
		{"breakdown", Technology::Modifier::Modifies::BreakdownRate},
		{"education", Technology::Modifier::Modifies::EducationEfficiency},
		{"maintenance_cost", Technology::Modifier::Modifies::MaintenanceCost},
		{"pop_fertility", Technology::Modifier::Modifies::PopulationFertility},
		{"pop_morale", Technology::Modifier::Modifies::PopulationMorale},
		{"pop_mortality", Technology::Modifier::Modifies::PopulationMortality},
		{"recycling", Technology::Modifier::Modifies::RecyclingEfficiency},
		{"smelter", Technology::Modifier::Modifies::SmelterEfficiency},
		{"structure_cost", Technology::Modifier::Modifies::StructureCost},
		{"structure_decay", Technology::Modifier::Modifies::StructureDecay}
	};


	std::map<std::string, Technology::Unlock::Unlocks> StringToUnlock =
	{
		{"disaster_prediction", Technology::Unlock::Unlocks::DisasterPrediction},
		{"robot", Technology::Unlock::Unlocks::Robot},
		{"satellite", Technology::Unlock::Unlocks::Satellite},
		{"structure", Technology::Unlock::Unlocks::Structure},
		{"vehicle", Technology::Unlock::Unlocks::Vehicle}
	};


	void verifySubElementTypes(const NAS2D::Xml::XmlElement& parentElement, const std::vector<std::string>& allowedNames, const std::string& errorMessagePrefix)
	{
		for (auto subElement = parentElement.firstChildElement(); subElement; subElement = subElement->nextSiblingElement())
		{
			const auto& elementName = subElement->value();
			if (std::find(allowedNames.begin(), allowedNames.end(), elementName) == allowedNames.end())
			{
				throw std::runtime_error(errorMessagePrefix + "Unknown element '" + elementName + "' at (line " + std::to_string(subElement->row()) + ", column " + std::to_string(subElement->column()) + ")");
			}
		}
	}


	template <typename UnaryOperation>
	auto readSubElementArray(const NAS2D::Xml::XmlElement& parentElement, const std::string& subElementName, UnaryOperation mapFunction)
	{
		using ResultType = decltype(mapFunction(std::declval<NAS2D::Xml::XmlElement&>()));
		using ElementType = std::remove_cv_t<std::remove_reference_t<ResultType>>;

		std::vector<ElementType> results;
		for (auto subElement = parentElement.firstChildElement(subElementName); subElement; subElement = subElement->nextSiblingElement(subElementName))
		{
			results.push_back(mapFunction(*subElement));
		}
		return results;
	}


	void readEffects(NAS2D::Xml::XmlElement& effects, Technology& technology)
	{
		verifySubElementTypes(effects, {"modifier", "unlock"}, "TechnologyReader: ");

		technology.modifiers = readSubElementArray(effects, "modifier", [](auto& element) {
			return Technology::Modifier{StringToModifier.at(element.attribute("type")), std::stof(element.getText())};
		});
		technology.unlocks = readSubElementArray(effects, "unlock", [](auto& element) {
			return Technology::Unlock{StringToUnlock.at(element.attribute("type")), element.getText()};
		});
	}


	Technology readTechnology(NAS2D::Xml::XmlElement& technology)
	{
		const auto attributes = NAS2D::attributesToDictionary(technology);
		Technology tech = {
			attributes.get<int>("id"),
			attributes.get<int>("lab_type"),
			attributes.get<int>("cost")
		};

		for (auto techElement = technology.firstChildElement(); techElement; techElement = techElement->nextSiblingElement())
		{
			const std::string elementName = techElement->value();
			std::string elementValue = techElement->getText();
			if (elementName == "name")
			{
				tech.name = elementValue;
			}
			else if (elementName == "description")
			{
				tech.description = elementValue;
			}
			else if (elementName == "requires")
			{
				auto requiredIds = NAS2D::split(elementValue);
				for (auto& id : requiredIds)
				{
					tech.requiredTechnologies.push_back(std::stoi(id));
				}
			}
			else if (elementName == "effects")
			{
				readEffects(*techElement, tech);
			}
			else
			{
				throw std::runtime_error("TechnologyReader: Unknown element '" + elementName + "' at (" + std::to_string(techElement->row()) + ", " + std::to_string(techElement->column()) + ")");
			}
		}

		return tech;
	}
}


TechnologyCatalog::TechnologyCatalog(const std::string& techFile)
{
	Xml::XmlDocument xmlDocument = openXmlFile(techFile, "technology");

	auto root = xmlDocument.firstChildElement("technology");

	auto firstCategory = root->firstChildElement("category");
	if (!firstCategory) { return; }
	readCategories(*firstCategory);
}


const Technology& TechnologyCatalog::technologyFromId(int id) const
{
	for (const auto& category : mCategories)
	{
		const auto& techList = category.second;
		const auto it = std::find_if(techList.begin(), techList.end(), [id](const Technology& tech) { return tech.id == id; });

		if (it != techList.end())
		{
			return (*it);
		}
	}

	throw std::runtime_error("TechnologyReader: Requested technology id '" + std::to_string(id) + "' not found.");
}


const std::vector<Technology> TechnologyCatalog::technologiesInCategory(const std::string& categoryName) const
{
	return mCategories.at(categoryName);
}


void TechnologyCatalog::readCategories(NAS2D::Xml::XmlElement& node)
{
	for (auto category = &node; category; category = category->nextSiblingElement())
	{
		const auto attributes = NAS2D::attributesToDictionary(*category);
		const std::string name = attributes.get("name");

		auto it = mCategories.find(name);
		if (it != mCategories.end())
		{
			throw std::runtime_error("TechnologyReader: Category redefinition '" + name +
				"' at (" + std::to_string(category->row()) + ", " + std::to_string(category->column()) + ")");
		}
		readTechnologiesInCategory(name, *category);
		mCategorNames.push_back(name);
	}
}


void TechnologyCatalog::readTechnologiesInCategory(const std::string& categoryName, NAS2D::Xml::XmlElement& category)
{
	for (auto technologyNode = category.firstChildElement(); technologyNode; technologyNode = technologyNode->nextSiblingElement())
	{
		Technology tech = readTechnology(*technologyNode);

		auto& technologies = mCategories[categoryName];
		const auto it = std::find_if(technologies.begin(), technologies.end(), [tech](const Technology& technology) { return technology.id == tech.id; });
		if (it != technologies.end())
		{
			throw std::runtime_error("TechnologyReader: Technology ID redefinition '" + std::to_string(tech.id) +
				"' at (" + std::to_string(technologyNode->row()) + ", " + std::to_string(technologyNode->column()) + ")");
		}

		technologies.push_back(tech);
	}
}
