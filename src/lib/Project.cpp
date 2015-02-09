#include "Project.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>

#include "ApplicationSettings.h"
#include "data/access/GraphAccessProxy.h"
#include "data/access/LocationAccessProxy.h"
#include "data/graph/Token.h"
#include "data/parser/cxx/CxxParser.h"
#include "utility/logging/logging.h"
#include "utility/messaging/type/MessageFinishedParsing.h"
#include "utility/file/FileSystem.h"

std::shared_ptr<Project> Project::create(GraphAccessProxy* graphAccessProxy, LocationAccessProxy* locationAccessProxy)
{
	std::shared_ptr<Project> ptr(new Project(graphAccessProxy, locationAccessProxy));
	ptr->clearStorage();
	return ptr;
}

Project::~Project()
{
}

bool Project::loadProjectSettings(const std::string& projectSettingsFile)
{
	bool success = ProjectSettings::getInstance()->load(projectSettingsFile);
	if(success)
	{
		m_projectSettingsFilepath = projectSettingsFile;
	}
	return success;
}

bool Project::saveProjectSettings( const std::string& projectSettingsFile )
{
	if(!projectSettingsFile.empty())
	{
		m_projectSettingsFilepath = projectSettingsFile;
		ProjectSettings::getInstance()->save(projectSettingsFile);
	}
	else if (!m_projectSettingsFilepath.empty())
	{
		ProjectSettings::getInstance()->save(m_projectSettingsFilepath);
	}
	else
	{
		return false;
	}
	LOG_INFO_STREAM(<< "Projectsettings saved in File: " << m_projectSettingsFilepath);
	return true;
}

void Project::clearProjectSettings()
{
	m_projectSettingsFilepath.clear();
	ProjectSettings::getInstance()->clear();
}

bool Project::setSourceDirectoryPath(const std::string& sourceDirectoryPath)
{
	m_projectSettingsFilepath = sourceDirectoryPath + "/ProjectSettings.xml";
	return ProjectSettings::getInstance()->setSourcePath(sourceDirectoryPath);
}

void Project::clearStorage()
{
	m_storage = std::make_shared<Storage>();

	m_graphAccessProxy->setSubject(m_storage.get());
	m_locationAccessProxy->setSubject(m_storage.get());

	Token::resetNextId();
}

void Project::parseCode()
{
	std::string sourcePath = ProjectSettings::getInstance()->getSourcePath();
	if (sourcePath.size())
	{
		std::vector<std::string> includePaths;
		includePaths.push_back(sourcePath);
		if (!m_fileManager)
		{
			std::vector<std::string> sourcePaths;
			sourcePaths.push_back(sourcePath);
			std::vector<std::string> sourceExtensions;
			sourceExtensions.push_back(".cpp");
			sourceExtensions.push_back(".cc");
			std::vector<std::string> includeExtensions;
			includeExtensions.push_back(".h");
			includeExtensions.push_back(".hpp");
			m_fileManager = std::make_shared<FileManager>(sourcePaths, includePaths, sourceExtensions, includeExtensions); // todo: move this creation to another place (after projectsettings have been loaded)
		}

		m_fileManager->fetchFilePaths();
		std::vector<std::string> addedFilePaths = m_fileManager->getAddedFilePaths();
		std::vector<std::string> updatedFilePaths = m_fileManager->getUpdatedFilePaths();
		std::vector<std::string> removedFilePaths = m_fileManager->getRemovedFilePaths();

		m_storage->clearFileData(addedFilePaths);
		m_storage->clearFileData(updatedFilePaths);
		m_storage->clearFileData(removedFilePaths);

		std::vector<std::string> filesToParse = addedFilePaths;
		for (std::string updatedFilePath: updatedFilePaths)
		{
			filesToParse.push_back(updatedFilePath);
		}

		if (filesToParse.size() == 0)
		{
			MessageFinishedParsing(0, 0, m_storage->getErrorCount()).dispatch();
		}
		else
		{
			// Add the SourcePaths as HeaderSearchPaths as well, so clang will also look here when searching include files.
			std::vector<std::string> headerSearchPaths = ProjectSettings::getInstance()->getHeaderSearchPaths();
			for (int i = 0; i < includePaths.size(); i++)
			{
				headerSearchPaths.push_back(includePaths[i]);
			}

			CxxParser parser(m_storage.get(), m_fileManager.get());
			clock_t time = clock();
			parser.parseFiles(
				filesToParse,
				ApplicationSettings::getInstance()->getHeaderSearchPaths(),
				headerSearchPaths
			);
			time = clock() - time;

			// m_storage->logGraph();
			// m_storage->logLocations();

			double parseTime = (double)(time) / CLOCKS_PER_SEC;
			LOG_INFO_STREAM(<< "parse time: " << parseTime);

			MessageFinishedParsing(filesToParse.size(), parseTime, m_storage->getErrorCount()).dispatch();
		}
	}
}

Project::Project(GraphAccessProxy* graphAccessProxy, LocationAccessProxy* locationAccessProxy)
	: m_graphAccessProxy(graphAccessProxy)
	, m_locationAccessProxy(locationAccessProxy)
{
}
