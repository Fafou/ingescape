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

#include "experimentationm.h"
#include <misc/ingescapeutils.h>

/**
 * @brief Constructor
 * @param name
 * @param creationDate
 * @param parent
 */
ExperimentationM::ExperimentationM(CassUuid cassUuid,
                                   QString name,
                                   QDateTime creationDate,
                                   QObject *parent) : QObject(parent),
    _uid(""),
    _name(name),
    _creationDate(creationDate),
    _cassUuid(cassUuid)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    char chrCassUid[CASS_UUID_STRING_LENGTH];
    cass_uuid_string(_cassUuid, chrCassUid);
    _uid = QString(chrCassUid);

    qInfo() << "New Model of Experimentation" << _name << "created" << _creationDate.toString("dd/MM/yy hh:mm:ss") << "(" << _uid << ")";


    // Record setups are sorted on their start date/time (chronological order)
    _allRecordSetups.setSortProperty("startDateTime");


    // Characteristic "Subject Name"
    //CharacteristicM* characteristicName = new CharacteristicM(CHARACTERISTIC_SUBJECT_ID, CharacteristicValueTypes::TEXT, true, this);
    //_allCharacteristics.append(characteristicName);


    //
    // FIXME for tests
    //

    /*SubjectM* debugSubject = new SubjectM("S", this);
    debugSubject->mapCharacteristicValues()->insert(CHARACTERISTIC_SUBJECT_ID, QVariant("M. JEAN"));

    _allSubjects.append(debugSubject);


    // Directory for platform files
    QString platformPath = IngeScapeUtils::getPlatformsPath();

    for (int i = 0; i < 2; i++)
    {
        QString taskName = QString("Task for test %1").arg(i + 1);

        TaskM* debugTask = new TaskM(taskName, this);

        QString platformFilePath = QString("%1i2.json").arg(platformPath);
        QUrl platformFileUrl = QUrl(platformFilePath);
        debugTask->setplatformFileUrl(platformFileUrl);

        _allTasks.append(debugTask);
    }*/
}


/**
 * @brief Destructor
 */
ExperimentationM::~ExperimentationM()
{
    qInfo() << "Delete Model of Experimentation" << _name << "created" << _creationDate.toString("dd/MM/yy hh:mm:ss") << "(" << _uid << ")";

    // Delete all characteristics of our experimentation
    _allCharacteristics.deleteAllItems();

    // Delete all subjects of our experimentation
    _allSubjects.deleteAllItems();

    // Delete all tasks of our experimentation
    _allTasks.deleteAllItems();

    // Delete all record setups of our experimentation
    _allRecordSetups.deleteAllItems();

}


/**
 * @brief Get the unique identifier in Cassandra Data Base
 * @return
 */
CassUuid ExperimentationM::getCassUuid()
{
    return _cassUuid;
}


/**
 * @brief Add a characteristic to our experimentation
 * @param characteristic
 */
void ExperimentationM::addCharacteristic(CharacteristicM* characteristic)
{
    if ((characteristic != nullptr) && !_hashFromUIDtoCharacteristic.contains(characteristic->uid()))
    {
        // Add to the hash
        _hashFromUIDtoCharacteristic.insert(characteristic->uid(), characteristic);

        // Add to the list
        _allCharacteristics.append(characteristic);

        // Add this characteristic for all existing subjects
        for (SubjectM* subject : _allSubjects)
        {
            if (subject != nullptr)
            {
                subject->addCharacteristic(characteristic);
            }
        }
    }
}


/**
 * @brief Remove a characteristic from our experimentation
 * @param characteristic
 */
void ExperimentationM::removeCharacteristic(CharacteristicM* characteristic)
{
    if ((characteristic != nullptr) && _hashFromUIDtoCharacteristic.contains(characteristic->uid()))
    {
        // Remove from the hash
        _hashFromUIDtoCharacteristic.remove(characteristic->uid());

        // Remove from the list
        _allCharacteristics.remove(characteristic);

        // Remove this characteristic for all existing subjects
        for (SubjectM* subject : _allSubjects)
        {
            if (subject != nullptr)
            {
                subject->removeCharacteristic(characteristic);
            }
        }
    }
}


/**
 * @brief Add a subject to our experimentation
 * @param subject
 */
void ExperimentationM::addSubject(SubjectM* subject)
{
    if (subject != nullptr)
    {
        // Add to the list
        _allSubjects.append(subject);
    }
}


/**
 * @brief Remove a subject from our experimentation
 * @param subject
 */
void ExperimentationM::removeSubject(SubjectM* subject)
{
    if (subject != nullptr)
    {
        // Remove from the list
        _allSubjects.remove(subject);
    }
}


/**
 * @brief Add a task to our experimentation
 * @param task
 */
void ExperimentationM::addTask(TaskM* task)
{
    if (task != nullptr)
    {
        // Add to the list
        _allTasks.append(task);
    }
}


/**
 * @brief Remove a task from our experimentation
 * @param task
 */
void ExperimentationM::removeTask(TaskM* task)
{
    if (task != nullptr)
    {
        // Remove from the list
        _allTasks.remove(task);
    }
}


/**
 * @brief Add a record setup to our experimentation
 * @param record
 */
void ExperimentationM::addRecordSetup(RecordSetupM* recordSetup)
{
    if (recordSetup != nullptr)
    {
        // Add to the list
        _allRecordSetups.append(recordSetup);

        if (recordSetup->subject() != nullptr)
        {
            qDebug() << "Subject:" << recordSetup->subject()->displayedId();

            for (CharacteristicM* characteristic : _allCharacteristics)
            {
                if ((characteristic != nullptr) && recordSetup->subject()->mapCharacteristicValues()->contains(characteristic->name()))
                {
                    qDebug() << characteristic->name() << ":" << recordSetup->subject()->mapCharacteristicValues()->value(characteristic->name());
                }
            }
        }
    }
}


/**
 * @brief Remove a record setup from our experimentation
 * @param record
 */
void ExperimentationM::removeRecordSetup(RecordSetupM* recordSetup)
{
    if (recordSetup != nullptr)
    {
        // Remove from the list
        _allRecordSetups.remove(recordSetup);
    }
}


/**
 * @brief Get a characteristic from its UID
 * @param uid
 * @return
 */
CharacteristicM* ExperimentationM::getCharacteristicFromUID(QString uid)
{
    if (_hashFromUIDtoCharacteristic.contains(uid)) {
        return _hashFromUIDtoCharacteristic.value(uid);
    }
    else {
        return nullptr;
    }
}

