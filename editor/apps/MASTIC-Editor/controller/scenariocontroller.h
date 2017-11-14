
/*
 *	ScenarioController
 *
 *  Copyright (c) 2016-2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *
 */

#ifndef SCENARIOCONTROLLER_H
#define SCENARIOCONTROLLER_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"
#include "viewModel/actionvm.h"
#include "viewModel/agentinmappingvm.h"
#include "viewModel/actioninpalettevm.h"
#include "controller/actioneditorcontroller.h"
#include "masticmodelmanager.h"


/**
 * @brief The ScenarioController class defines the main controller of our application
 */
class ScenarioController: public QObject
{
    Q_OBJECT

    // Filtered Sorted list of actions
    I2_QOBJECT_LISTMODEL_WITH_SORTFILTERPROXY(ActionM, actionsList)

    // Sorted list of ations by start time
    I2_QOBJECT_LISTMODEL(ActionEditorController, openedActionsEditorsControllers)

    // Selected action
    I2_QML_PROPERTY_DELETE_PROOF(ActionM*, selectedAction)

    // Used for action edition
    // --- List of comparisons values types
    I2_ENUM_LISTMODEL(ActionComparisonValueType, comparisonsValuesTypesList)
    // --- List of comparisons state types
    I2_ENUM_LISTMODEL(ActionComparisonValueType, comparisonsAgentsTypesList)
    // --- List of validity duration type
    I2_ENUM_LISTMODEL(ValidationDurationType, validationDurationsTypesList)
    // --- List of effects states type
    I2_ENUM_LISTMODEL(ActionEffectValueType, effectsAgentsTypesList)
    // --- List of effects links type
    I2_ENUM_LISTMODEL(ActionEffectValueType, effectsLinksTypesList)

    // --- List of conditions type
    I2_ENUM_LISTMODEL(ActionConditionType, conditionsTypesList)
    // --- List of effects type
    I2_ENUM_LISTMODEL(ActionEffectType, effectsTypesList)

    // --- agents list in mapping
    I2_QOBJECT_LISTMODEL(AgentInMappingVM, agentsInMappingList)


    // List of actions in palette
    I2_QOBJECT_LISTMODEL(ActionInPaletteVM, actionsInPaletteList)

    // List of actions in timeline
    I2_QOBJECT_LISTMODEL(ActionVM, actionsInTimeLine)

public:

    /**
     * @brief Default constructor
     * @param scenarios files path
     * @param parent
     */
    explicit ScenarioController(QString scenariosPath, QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~ScenarioController();

    /**
      * @brief Open the action editor
      * @param action model
      */
    Q_INVOKABLE void openActionEditor(ActionM* actionM);

    /**
      * @brief Delete an action from the list
      * @param action model
      */
    Q_INVOKABLE void deleteAction(ActionM * actionM);

    /**
      * @brief Valide action edition
      * @param action editor controller
      */
    Q_INVOKABLE void valideActionEditor(ActionEditorController* actionEditorC);

    /**
      * @brief Close action edition
      * @param action editor controller
      */
    Q_INVOKABLE void closeActionEditor(ActionEditorController* actionEditorC);

    /**
     * @brief Set an action into the palette at index
     * @param index where to insert the action
     * @param action to insert
     */
    Q_INVOKABLE void setActionInPalette(int index, ActionM* actionM);

    /**
     * @brief Import a scenario a file (actions, palette, timeline actions )
     */
    Q_INVOKABLE void importScenarioFromFile();

    /**
     * @brief Export a scenario to a selected file (actions, palette, timeline actions)
     */
    Q_INVOKABLE void exportScenarioToSelectedFile();

Q_SIGNALS:


public Q_SLOTS:

    /**
      * @brief slot on agents in mapping list count change
      */
    void onAgentsInMappingListCountChange();

private :
    /**
     * @brief Get a new action name
     */
    QString _buildNewActionName();

    /**
     * @brief Import the scenario from JSON file
     * @param scenarioFilePath
     */
    void _importScenarioFromFile(QString scenarioFilePath);

    /**
     * @brief Export the scenario to JSON file
     * @param scenarioFilePath
     */
    void _exportScenarioToFile(QString scenarioFilePath);


protected:

    // Path to the directory containing JSON files to save scenarios
    QString _scenariosDirectoryPath;
    QString _scenariosDefaultFilePath;

    // Helper to manage JSON definitions of agents
    JsonHelper* _jsonHelper;

    // Map of actions editors controllers from the actions view model
    QHash<ActionM*, ActionEditorController*> _mapActionsEditorControllersFromActionVM;

    // Map of actions model from the action name
    QHash<QString, ActionM*> _mapActionsFromActionName;
};

QML_DECLARE_TYPE(ScenarioController)

#endif // SCENARIOCONTROLLER_H
