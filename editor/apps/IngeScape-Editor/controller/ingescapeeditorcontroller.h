/*
 *	IngeScape Editor
 *
 *  Copyright © 2017-2018 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *      Alexandre Lemort <lemort@ingenuity.io>
 *      Bruno Lemenicier <lemenicier@ingenuity.io>
 *
 */

#ifndef INGESCAPEEDITORCONTROLLER_H
#define INGESCAPEEDITORCONTROLLER_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include <I2PropertyHelpers.h>

#include <controller/abstracttimeactionslinescenarioviewcontroller.h>
#include <controller/agentsmappingcontroller.h>
#include <controller/agentssupervisioncontroller.h>
#include <controller/hostssupervisioncontroller.h>
#include <controller/ingescapelaunchermanager.h>
#include <controller/ingescapemodelmanager.h>
#include <controller/logstreamcontroller.h>
#include <controller/networkcontroller.h>
#include <controller/recordssupervisioncontroller.h>
#include <controller/scenariocontroller.h>
#include <controller/valueshistorycontroller.h>

#include <misc/terminationsignalwatcher.h>


/**
 * @brief The IngeScapeEditorController class defines the main controller of the INGESCAPE editor
 */
class IngeScapeEditorController : public QObject
{
    Q_OBJECT

    // Network settings - network device
    I2_QML_PROPERTY_READONLY(QString, networkDevice)

    // Network settings - ip address
    I2_QML_PROPERTY_READONLY(QString, ipAddress)

    // Network settings - port
    I2_QML_PROPERTY_READONLY(int, port)

    // Error message when a connection attempt fails
    I2_QML_PROPERTY_READONLY(QString, errorMessageWhenConnectionFailed)

    // Snapshot Directory
    I2_QML_PROPERTY_READONLY(QString, snapshotDirectory)

    // Manager for the data model of our INGESCAPE editor
    I2_QML_PROPERTY_READONLY(IngeScapeModelManager*, modelManager)

    // Controller for agents supervision
    I2_QML_PROPERTY_READONLY(AgentsSupervisionController*, agentsSupervisionC)

    // Controller for hosts supervision
    I2_QML_PROPERTY_READONLY(HostsSupervisionController*, hostsSupervisionC)

    // Controller for records supervision
    I2_QML_PROPERTY_READONLY(RecordsSupervisionController*, recordsSupervisionC)

    // Controller for agents mapping
    I2_QML_PROPERTY_READONLY(AgentsMappingController*, agentsMappingC)

    // Controller for network communication
    I2_QML_PROPERTY_READONLY(NetworkController*, networkC)

    // Controller for scenario management
    I2_QML_PROPERTY_READONLY(ScenarioController*, scenarioC)

    // Controller for the history of values
    I2_QML_PROPERTY_READONLY(ValuesHistoryController*, valuesHistoryC)

    // Controller for the time line
    I2_QML_PROPERTY_READONLY(AbstractTimeActionslineScenarioViewController*, timeLineC)

    // Manager for launchers of INGESCAPE agents
    I2_QML_PROPERTY_READONLY(IngeScapeLauncherManager*, launcherManager)

    // Opened log stream viewers
    I2_QOBJECT_LISTMODEL(LogStreamController, openedLogStreamControllers)


public:

    /**
     * @brief Constructor
     * @param parent
     */
    explicit IngeScapeEditorController(QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~IngeScapeEditorController();


    /**
     * @brief Method used to provide a singleton to QML
     * @param engine
     * @param scriptEngine
     * @return
     */
    static QObject* qmlSingleton(QQmlEngine* engine, QJSEngine* scriptEngine);


    /**
      * @brief Open a platform file (actions, palette, timeline actions, mappings)
      */
    Q_INVOKABLE void openPlatformFromFile();


    /**
      * @brief Save a platform to a selected file (actions, palette, timeline actions, mappings)
      */
    Q_INVOKABLE void savePlatformToSelectedFile();


    /**
      * @brief Save a platform to the default file (actions, palette, timeline actions, mappings)
      */
    void savePlatformToDefaultFile();


    /**
      * @brief Create a new platform (actions, palette, timeline actions, mappings)
      *        by deleting all existing data
      */
    Q_INVOKABLE void createNewPlatform();


    /**
      * @brief Actions to perform before the application closing
      */
    Q_INVOKABLE void processBeforeClosing();


    /**
      * @brief Check if we can delete an agent (view model) from the list in supervision
      *        Check dependencies in the mapping and in the actions (conditions, effects)
      * @param agentName
      */
    Q_INVOKABLE bool canDeleteAgentFromSupervision(QString agentName);


    /**
      * @brief Check if we can delete an agent (in mapping) from the mapping view
      *        Check dependencies in the actions (conditions, effects)
      * @param agent in mapping to delete
      */
    Q_INVOKABLE bool canDeleteAgentInMapping(QString agentName);


    /**
     * @brief Re-Start the network with a port and a network device
     * @param strPort
     * @param networkDevice
     * @return true when success
     */
    Q_INVOKABLE bool restartNetwork(QString strPort, QString networkDevice);


    /**
      * @brief Close a definition
      * @param definition
      */
    Q_INVOKABLE void closeDefinition(DefinitionM* definition);


    /**
      * @brief Close an action editor
      * @param actionEditorC
      */
    Q_INVOKABLE void closeActionEditor(ActionEditorController* actionEditorC);


    /**
     * @brief Close a "Log Stream" controller
     * @param logStreamC
     */
    Q_INVOKABLE void closeLogStreamController(LogStreamController* logStreamC);


public Q_SLOTS:

    /**
      * @brief Method used to force the creation of our singleton from QML
      */
    void forceCreation();


    /**
      * @brief Get the position of the mouse cursor in global screen coordinates
      *
      * @remarks You must use mapToGlobal to convert it to local coordinates
      *
      * @return
      */
    QPointF getGlobalMousePosition();


    /**
     * @brief Slot called when we have to open the "Log Stream" of a list of agents
     * @param models
     */
    void onOpenLogStreamOfAgents(QList<AgentM*> models);


Q_SIGNALS:

    /**
      * @brief Reset the mapping and timeline views
      */
    void resetMappindAndTimeLineViews();


private:

    /**
      * @brief Open the platform from JSON file
      * @param platformFilePath
      */
    void _openPlatformFromFile(QString platformFilePath);


    /**
      * @brief Save the platform to JSON file
      * @param platformFilePath
      */
    void _savePlatformToFile(QString platformFilePath);


private:

    // To subscribe to termination signals
    TerminationSignalWatcher *_terminationSignalWatcher;

    // Helper to manage JSON files
    JsonHelper* _jsonHelper;

    // Path to the directory containing JSON files to save platform
    QString _platformDirectoryPath;
    QString _platformDefaultFilePath;

};

QML_DECLARE_TYPE(IngeScapeEditorController)

#endif // INGESCAPEEDITORCONTROLLER_H
