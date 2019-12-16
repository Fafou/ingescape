/*
 *	IngeScape Editor
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Natanael Vaugien    <vaugien@ingenuity.io>
 *
 */

#include "recordassessmentm.h"

#include "controller/assessmentsmodelmanager.h"

/**
 * @brief Record table name
 */
const QString RecordAssessmentM::table = "ingescape.record";

/**
 * @brief Record table column names
 */
const QStringList RecordAssessmentM::columnNames = {
    "id",
    "end_date",
    "end_time",
    "id_experimentation",
    "id_task_instance",
    "name_record",
    "offset_tl",
    "platform",
    "time_of_day",
    "year_month_day",
};

/**
 * @brief TaskInstance table primary keys IN ORDER
 */
const QStringList RecordAssessmentM::primaryKeys = {
    "id_task_instance",
};

RecordAssessmentM::RecordAssessmentM(QString uid,
                                     QString name,
                                     QDateTime beginDate,
                                     QDateTime endDate,
                                     QObject *parent) : RecordM(uid, name, beginDate, endDate, parent)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

/**
 * @brief Destructor
 */
RecordAssessmentM::~RecordAssessmentM()
{
    qInfo() << "Delete Model of Record" << _name << "(" << _uid << ") ";
}
/**
 * @brief Static factory method to create an experiment from a CassandraDB record
 * @param row
 * @return
 */
RecordAssessmentM* RecordAssessmentM::createFromCassandraRow(const CassRow* row)
{
    RecordAssessmentM* record = nullptr;

    if (row != nullptr)
    {
        CassUuid recordUid;
        cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &recordUid);
        QString uuidRecordString = AssessmentsModelManager::cassUuidToQString(recordUid);
        QString recordName = AssessmentsModelManager::getStringValueFromColumnName(row, "name");
        QDateTime startDateTime(AssessmentsModelManager::getDateTimeFromColumnNames(row, "creation_date", "creation_time"));
        QDateTime endDateTime(AssessmentsModelManager::getDateTimeFromColumnNames(row, "creation_date", "creation_time"));


        record = new RecordAssessmentM(uuidRecordString, recordName, startDateTime, endDateTime, nullptr);
    }

    return record;
}
