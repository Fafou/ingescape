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
 *
 */

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>
#include <QtQml>

#include <I2PropertyHelpers.h>


/**
  * Types of IngeScape elements on the network
  */
I2_ENUM(IngeScapeTypes, UNKNOWN, AGENT, LAUNCHER, RECORDER, EDITOR, ASSESSMENTS)


/**
  * Types:
  * - Agent Input
  * - Agent Output
  * - Agent Parameter
  */
I2_ENUM(AgentIOPTypes, INPUT, OUTPUT, PARAMETER)


/**
  * Types of the value of an Agent Input / Output / Parameter
  */
I2_ENUM(AgentIOPValueTypes, INTEGER = 1, DOUBLE, STRING, BOOL, IMPULSION, DATA, MIXED, UNKNOWN)


/**
  * Groups for types of the value of an Agent Input / Output / Parameter
  */
I2_ENUM(AgentIOPValueTypeGroups, NUMBER, STRING, IMPULSION, DATA, MIXED, UNKNOWN)


/**
  * Types of log
  */
I2_ENUM_CUSTOM(LogTypes, IGS_LOG_TRACE, IGS_LOG_DEBUG, IGS_LOG_INFO, IGS_LOG_WARNING, IGS_LOG_ERROR, IGS_LOG_FATAL)


static const QString SEPARATOR_AGENT_NAME_AND_IOP = QString("##");
static const QString SEPARATOR_IOP_NAME_AND_IOP_VALUE_TYPE = QString("::");
static const QString SEPARATOR_LINK_OUTPUT_AND_LINK_INPUT = QString("-->");
static const QString HOSTNAME_NOT_DEFINED = QString("HOSTNAME NOT DEFINED");


static const QString command_LoadDefinition = "LOAD_THIS_DEFINITION#";
static const QString command_LoadMapping = "LOAD_THIS_MAPPING#";
static const QString command_ClearMapping = "CLEAR_MAPPING";

static const QString command_StartAgent = "RUN";
static const QString command_StopAgent = "STOP";
static const QString command_MuteAgent = "MUTE_ALL";
static const QString command_UnmuteAgent = "UNMUTE_ALL";
static const QString command_MuteAgentOutput = "MUTE";
static const QString command_UnmuteAgentOutput = "UNMUTE";
static const QString command_FreezeAgent = "FREEZE";
static const QString command_UnfreezeAgent = "UNFREEZE";

static const QString command_MapAgents = "MAP";
static const QString command_UnmapAgents = "UNMAP";

static const QString command_EnableLogStream = "ENABLE_LOG_STREAM";
static const QString command_DisableLogStream = "DISABLE_LOG_STREAM";
static const QString command_EnableLogFile = "ENABLE_LOG_FILE";
static const QString command_DisableLogFile = "DISABLE_LOG_FILE";
static const QString command_SetLogPath = "SET_LOG_PATH";
static const QString command_SetDefinitionPath = "SET_DEFINITION_PATH";
static const QString command_SetMappingPath = "SET_MAPPING_PATH";
static const QString command_SaveDefinitionToPath = "SAVE_DEFINITION_TO_PATH";
static const QString command_SaveMappingToPath = "SAVE_MAPPING_TO_PATH";

static const QString command_ExecutedAction = "EXECUTED_ACTION";

static const QString command_StartToRecord = "START_RECORD";
static const QString command_StopToRecord = "STOP_RECORD";
static const QString command_DeleteRecord = "DELETE_RECORD";
static const QString command_LoadReplay = "LOAD_REPLAY";
static const QString command_StartReplay = "START_REPLAY"; //TODO: implement this command
static const QString command_StopTheReplay = "STOP_REPLAY";
static const QString command_PauseTheReplay = "PAUSE_REPLAY"; //TODO: implement this command


// Date date of our application
static const QDate APPLICATION_START_DATE = QDate::currentDate();


/*!
 * Define a QTime and a QDateTime: Manage a date in addition to the time
 * to prevent a delta in hours between JS (QML) and C++.
 * The QDateTime is readable/writable from QML.
 * The date used is the current date.
 */
#define I2_QML_PROPERTY_QTime(name) \
        Q_PROPERTY (QDateTime name READ qmlGet##name WRITE qmlSet##name NOTIFY name##Changed) \
    public: \
        QDateTime qmlGet##name () const { \
            return QDateTime(_##name##_Date, _##name); \
        } \
        QTime name () const { \
            return _##name; \
        } \
        virtual bool qmlSet##name (QDateTime value) { \
            bool hasChanged = false; \
            if (_##name != value.time()) { \
                _##name = value.time(); \
                _##name##_Date = value.date(); \
                hasChanged = true; \
                Q_EMIT name##Changed(value); \
            } \
            return hasChanged; \
        } \
        void set##name(QTime value) { \
            if (_##name != value) { \
                _##name = value; \
                Q_EMIT name##Changed(QDateTime(_##name##_Date, _##name)); \
            } \
        } \
    Q_SIGNALS: \
        void name##Changed (QDateTime value); \
    protected: \
        QTime _##name; \
        QDate _##name##_Date = APPLICATION_START_DATE;



/**
 * @brief The Enums class is a helper for general enumerations and constants
 */
class Enums : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit Enums(QObject *parent = nullptr);


    /**
     * @brief Get the group for an Agent Input/Output/Parameter Value Type
     * @param valueType
     * @return
     */
    static AgentIOPValueTypeGroups::Value getGroupForAgentIOPValueType(AgentIOPValueTypes::Value valueType);


    /**
     * @brief Get a displayable value: convert a variant into a string (in function of the value type)
     * @param valueType
     * @param value
     * @return
     */
    static QString getDisplayableValue(AgentIOPValueTypes::Value valueType, QVariant value);

    static QString getDisplayableValueFromInteger(int value);

    static QString getDisplayableValueFromDouble(double value);

    static QString getDisplayableValueFromString(QString value);

    static QString getDisplayableValueFromBool(bool value);

    static QString getDisplayableValueFromData(QByteArray value);


    /*static int getIntegerFromValue(QVariant value);

    static double getDoubleFromValue(QVariant value);

    static QString getStringFromValue(QVariant value);

    static bool getBoolFromValue(QVariant value);

    static QByteArray getDataFromValue(QVariant value);*/

};

QML_DECLARE_TYPE(Enums)

#endif // ENUMS_H
