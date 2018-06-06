/*
 *	IngeScape Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef MAPPINGEFFECTM_H
#define MAPPINGEFFECTM_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"

#include <model/scenario/effect/actioneffectm.h>


/**
 * Values of effect on mapping
 */
I2_ENUM_CUSTOM(MappingEffectValues, MAPPED, UNMAPPED)


/**
 * @brief The MappingEffectM class defines an action effect on a mapping between two iop value
 */
class MappingEffectM: public ActionEffectM
{
    Q_OBJECT

    // Value of our effect on mapping
    I2_QML_PROPERTY(MappingEffectValues::Value, mappingEffectValue)

    // Output Agent
    I2_QML_PROPERTY_CUSTOM_SETTER(AgentInMappingVM*, outputAgent)

    // Output of the output agent
    I2_QML_PROPERTY_CUSTOM_SETTER(AgentIOPM*, output)

    // Output name of the output agent
    I2_QML_PROPERTY(QString, outputName)

    // Input of the input agent
    I2_QML_PROPERTY_CUSTOM_SETTER(AgentIOPM*, input)

    // Input name of the input agent
    I2_QML_PROPERTY(QString, inputName)

    // List of outputs
    I2_QOBJECT_LISTMODEL(AgentIOPM, outputsList)

    // List of inputs
    I2_QOBJECT_LISTMODEL(AgentIOPM, inputsList)


public:

    /**
     * @brief Constructor
     * @param parent
     */
    explicit MappingEffectM(QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~MappingEffectM();


    /**
      * @brief Redefinition of action effect copy
      */
    void copyFrom(ActionEffectM* effect) Q_DECL_OVERRIDE;


    /**
    * @brief Setter for property "Agent"
    * @param agent
    */
    void setagent(AgentInMappingVM* agent) Q_DECL_OVERRIDE;


    /**
     * @brief Get a pair with the agent and the command (with parameters) of our effect
     * @return
     */
    QPair<AgentInMappingVM*, QStringList> getAgentAndCommandWithParameters() Q_DECL_OVERRIDE;


    /**
     * @brief Get a pair with the agent name and the reverse command (with parameters) of our effect
     * @return
     */
    QPair<QString, QStringList> getAgentNameAndReverseCommandWithParameters() Q_DECL_OVERRIDE;


Q_SIGNALS:


protected Q_SLOTS:

    /**
     * @brief Called when our "output agent" is destroyed
     * @param sender
     */
    void _onOutputAgentDestroyed(QObject* sender);


    /**
     * @brief Called when our input is destroyed
     * @param sender
     */
    void _onInputDestroyed(QObject* sender);


    /**
     * @brief Called when our output is destroyed
     * @param sender
     */
    void _onOutputDestroyed(QObject* sender);


    /**
      * @brief Slot called when the models of Inputs/Outputs/Parameters changed of the "Input agent (in mapping)"
      */
    void _onModelsOfIOPofInputAgentChanged();


    /**
      * @brief Slot called when the models of Inputs/Outputs/Parameters changed of the "Output agent (in mapping)"
      */
    void _onModelsOfIOPofOutputAgentChanged();


private:

    /**
    * @brief Update the selected input
    */
    void _updateInputSelected();


    /**
    * @brief Update the selected output
    */
    void _updateOutputSelected();


};

QML_DECLARE_TYPE(MappingEffectM)

#endif // MAPPINGEFFECTM_H
