/*
 *	IngeScape Assessments
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef SESSION_M_H
#define SESSION_M_H

#include <QObject>
#include <I2PropertyHelpers.h>

#include <model/subject/subjectm.h>
#include <model/protocol/protocolm.h>
#include <model/assessmentsenums.h>
#include <model/recordassessmentm.h>


/**
 * @brief The SessionM class defines a model of task instance
 */
class SessionM : public QObject
{
    Q_OBJECT

    // Name of our session
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, name)

    // User comments
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, comments)

    // Subject of our session
    I2_QML_PROPERTY_DELETE_PROOF(SubjectM*, subject)

    // Protocol of our session
    I2_QML_PROPERTY_DELETE_PROOF(ProtocolM*, task)

    // Start date and time of our session
    I2_QML_PROPERTY(QDateTime, startDateTime)

    // End date and time of our session
    I2_QML_PROPERTY_CUSTOM_SETTER(QDateTime, endDateTime)

    // Duration of our session
    I2_QML_PROPERTY_QTime(duration)
    //I2_QML_PROPERTY(QDateTime, duration)

    // Values of the independent variables of the task
    // "Qml Property Map" allows to set key-value pairs that can be used in QML bindings
    I2_QML_PROPERTY_READONLY(QQmlPropertyMap*, mapIndependentVariableValues)

    // List of records of our session
    I2_QOBJECT_LISTMODEL_WITH_SORTFILTERPROXY(RecordAssessmentM, recordsList)

    // Flag indicating if our session is recorded
    I2_QML_PROPERTY(bool, isRecorded)


public:

    /**
     * @brief Constructor
     * @param uid
     * @param name
     * @param subject
     * @param task
     * @param startDateTime
     * @param parent
     */
    explicit SessionM(CassUuid experimentationUuid,
                           CassUuid cassUuid,
                           QString name,
                           QString comments,
                           CassUuid subjectUuid,
                           CassUuid taskUuid,
                           QDateTime startDateTime,
                           QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~SessionM();

    /**
     * @brief TaskInstance table name
     */
    static const QString table;

    /**
     * @brief TaskInstance table column names
     */
    static const QStringList columnNames;

    /**
     * @brief TaskInstance table primary keys IN ORDER
     */
    static const QStringList primaryKeys;

    /**
     * @brief Accessor for the Cassandra UUID of this entry
     * @return
     */
    CassUuid getCassUuid() const { return _cassUuid; }

    /**
     * @brief Accessor for the task's Cassandra UUID of this entry
     * @return
     */
    CassUuid getTaskCassUuid() const { return _taskUuid; }

    /**
     * @brief Accessor for the subject's Cassandra UUID of this entry
     * @return
     */
    CassUuid getSubjectCassUuid() const { return _subjectUuid; }

    /**
     * @brief Set the value of the given independent variable into the QQmlPropertyMap
     * @param indepVar
     * @param value
     */
    void setIndependentVariableValue(IndependentVariableM* indepVar, const QString& value);

    /**
     * @brief Static factory method to create a task instance from a CassandraDB record
     * @param row
     * @return
     */
    static SessionM* createFromCassandraRow(const CassRow* row);

    /**
     * @brief Delete the given session from Cassandra DB
     * @param session
     */
    static void deleteSessionFromCassandra(const SessionM& session);

    /**
     * @brief Create a CassStatement to insert a SessionM into the DB.
     * The statement contains the values from the given taskInstance.
     * Passed taskInstance must have a valid and unique UUID.
     * @param taskInstance
     * @return
     */
    static CassStatement* createBoundInsertStatement(const SessionM& taskInstance);

    /**
     * @brief Create a CassStatement to update a SessionM into the DB.
     * The statement contains the values from the given taskInstance.
     * Passed taskInstance must have a valid and unique UUID.
     * @param taskInstance
     * @return
     */
    static CassStatement* createBoundUpdateStatement(const SessionM& taskInstance);


private Q_SLOTS:
    /**
     * @brief Slot called when a value of the Qml Property Map "map Independent Variable Values" changed
     * @param key
     * @param value
     */
    void _onIndependentVariableValueChanged(const QString& key, const QVariant& value);


private:
    /**
     * @brief For debug purpose: Print the value of all independent variables
     */
    void _printIndependentVariableValues();


private:
    QHash<QString, IndependentVariableM*> _mapIndependentVarByName;
    CassUuid _experimentationCassUuid;
    CassUuid _subjectUuid;
    CassUuid _taskUuid;

    CassUuid _cassUuid;


};

QML_DECLARE_TYPE(SessionM)

#endif // SESSION_M_H
