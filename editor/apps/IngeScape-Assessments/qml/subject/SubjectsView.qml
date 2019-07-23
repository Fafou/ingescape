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

import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import INGESCAPE 1.0

//import "theme" as Theme
import "../popup" as Popup


Item {
    id: rootItem

    //anchors.fill: parent


    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------

    property SubjectsController controller: null;

    property ExperimentationM experimentation: controller ? controller.currentExperimentation : null;

    property int indexPreviousSelectedSubject: -1;
    property int indexSubjectCurrentlyEditing: -1;


    //--------------------------------
    //
    //
    // Signals
    //
    //
    //--------------------------------

    // Close Subjects view
    signal closeSubjectsView();



    //--------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------

    /**
     * Edit a subject at a row index
     */
    function editSubjectAtRowIndex(rowIndex) {

        //console.log("QML: Edit the subject at the row index " + rowIndex);

        // Set the index
        rootItem.indexSubjectCurrentlyEditing = rowIndex;
    }


    /**
     * Stop the current edition of a subject
     */
    function stopCurrentEditionOfSubject() {
        if (rootItem.indexSubjectCurrentlyEditing > -1)
        {
            //console.log("QML: Stop the current edition of the subject at the row index " + rootItem.indexSubjectCurrentlyEditing);

            // Reset the index
            rootItem.indexSubjectCurrentlyEditing = -1;
        }
    }



    //--------------------------------------------------------
    //
    //
    // Content
    //
    //
    //--------------------------------------------------------

    Rectangle {
        id: background

        anchors.fill: parent

        color: IngeScapeTheme.veryDarkGreyColor
        border {
            color: IngeScapeTheme.darkGreyColor
            width: 1
        }
    }

    Button {
        id: btnClose

        anchors {
            right: parent.right
            top: parent.top
        }
        height: 30

        text: "X"

        onClicked: {
            //console.log("QML: close Subjects view");

            // Emit the signal "closeSubjectsView"
            rootItem.closeSubjectsView();
        }
    }

    Row {
        id: header

        anchors {
            top: parent.top
            topMargin: 10
            horizontalCenter: parent.horizontalCenter
        }
        height: 30

        spacing: 20

        Text {
            id: title

            text: "Subjects"

            height: parent.height
            verticalAlignment: Text.AlignVCenter

            color: IngeScapeTheme.whiteColor
            font {
                family: IngeScapeTheme.textFontFamily
                weight : Font.Medium
                pixelSize : 20
            }
        }

        Button {
            text: "New Subject"

            height: parent.height

            onClicked: {
                //console.log("QML: New Subject");

                if (rootItem.controller) {
                    rootItem.controller.createNewSubject();
                }
            }
        }
    }


    //
    // Characteristics Panel
    //
    Rectangle {
        id: characteristicsPanel

        anchors {
            left: parent.left
            top: header.bottom
            topMargin: 30
            bottom: parent.bottom
        }
        width: 300

        color: "transparent"
        border {
            color: IngeScapeTheme.darkGreyColor
            width: 1
        }

        Row {
            id: characteristicsHeader

            anchors {
                left: parent.left
                leftMargin: 5
                top: parent.top
                topMargin: 5
            }
            height: 30

            spacing: 20

            Text {
                id: titleCharacteristics

                text: "Characteristics"

                height: parent.height
                verticalAlignment: Text.AlignVCenter

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    weight: Font.Medium
                    pixelSize: 16
                }
            }

            Button {
                text: "New Characteristic"

                height: parent.height

                onClicked: {
                    // Open the popup
                    createCharacteristicPopup.open();
                }
            }
        }

        Column {
            anchors {
                top: characteristicsHeader.bottom
                topMargin: 20
                left: parent.left
                leftMargin: 5
                right: parent.right
                rightMargin: 5
            }

            Repeater {
                model: rootItem.experimentation ? rootItem.experimentation.allCharacteristics : null

                delegate: Characteristic {

                    modelM: model.QtObject

                    //
                    // Slots
                    //
                    onDeleteCharacteristic: {
                        if (rootItem.controller) {
                            rootItem.controller.deleteCharacteristic(model.QtObject);
                        }
                    }
                }
            }
        }
    }


    //
    // Subjects Panel
    //
    Rectangle {
        id: subjectsPanel

        anchors {
            left: characteristicsPanel.right
            right: parent.right
            top: header.bottom
            topMargin: 30
            bottom: parent.bottom
        }

        color: "transparent"
        border {
            color: IngeScapeTheme.darkGreyColor
            width: 1
        }


        TableView {
            id: tableSubjects

            anchors {
                fill: parent
                margins: 10
            }

            rowDelegate: Rectangle {
                width: childrenRect.width
                height: styleData.selected ? 30 : 20

                color: styleData.selected ? "lightblue"
                                          : (styleData.alternate ? "lightgray" : "white")
            }

            headerDelegate: Rectangle {
                height: 30
                width: parent.width

                color: "darkgray"

                Text {
                    id: txtColumnHeader

                    anchors {
                        fill: parent
                        leftMargin: (styleData.column === 0) ? 110 : 5
                    }
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: styleData.textAlignment

                    text: styleData.value
                    elide: Text.ElideRight
                    color: IngeScapeTheme.blackColor
                }

                Rectangle {
                    id: leftSeparator

                    visible: (styleData.column === 0)

                    anchors {
                        left: parent.left
                        leftMargin: 105
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: 1
                    color: "silver"
                }

                Rectangle {
                    id: rightSeparator

                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: 1
                    color: "silver"
                }
            }

            model: rootItem.experimentation ? rootItem.experimentation.allSubjects : null

            Instantiator {
                id: columnsInstantiator

                model: rootItem.experimentation ? rootItem.experimentation.allCharacteristics : null

                delegate: TableViewColumn {
                    id: column

                    property CharacteristicM characteristic: model.QtObject

                    role: model.name
                    title: model.name

                    width: (index === 0) ? 250 : 150

                    delegate: CharacteristicValueEditor {
                        id: characteristicValueEditor

                        characteristic: column.characteristic

                        characteristicValue: {
                            if (model) { console.log("column.role:" + column.role) }
                            model ? (column.role === "ID" ? model.displayedId : model.mapCharacteristicValues[column.role]) : ""
                        }

                        isSelected: styleData.selected

                        isCurrentlyEditing: (rootItem.indexSubjectCurrentlyEditing === styleData.row)


                        //
                        // Slots
                        //

                        onEditSubject: {
                            // Edit the subject at the row index
                            rootItem.editSubjectAtRowIndex(styleData.row);
                        }

                        onStopEditionOfSubject: {
                            // Stop the current edition of the subject
                            rootItem.stopCurrentEditionOfSubject();
                        }

                        onCharacteristicValueUpdated: {
                            if (model)
                            {
                                //console.log("QML: on Characteristic Value Updated " + value);

                                // Update the value (in C++)
                                model.mapCharacteristicValues[column.role] = value;
                            }
                        }

                        onDeleteSubject: {

                            if (rootItem.controller && model)
                            {
                                //console.log("QML: Delete Subject " + model.name);

                                // Stop the current edition of the subject
                                stopCurrentEditionOfSubject();

                                // Delete the subject
                                rootItem.controller.deleteSubject(model.QtObject);

                                // Clear the selection
                                tableSubjects.selection.clear();
                            }
                        }
                    }
                }

                onObjectAdded: {
                    //console.log("onObjectAdded " + index);
                    //tableSubjects.insertColumn(index, object);
                    tableSubjects.addColumn(object);
                }
                onObjectRemoved: {
                    //console.log("onObjectRemoved " + index);
                    tableSubjects.removeColumn(index);
                }
            }

            onDoubleClicked: {
                //console.log("onDoubleClicked " + row);

                // Edit the subject at the row index
                editSubjectAtRowIndex(row);
            }


            // NOT Called
            //onSelectionChanged: {
            //}
            // --> Workaround
            Connections {
                target: tableSubjects.selection

                onSelectionChanged: {
                    //console.log("onSelectionChanged:");

                    if ((rootItem.indexPreviousSelectedSubject === rootItem.indexSubjectCurrentlyEditing) && (rootItem.indexSubjectCurrentlyEditing > -1))
                    {
                        // Stop the current edition of the subject
                        stopCurrentEditionOfSubject();
                    }

                    // Reset
                    rootItem.indexPreviousSelectedSubject = -1;

                    tableSubjects.selection.forEach(function(rowIndex) {
                        //console.log("row " + rowIndex + " is selected");

                        // Update
                        rootItem.indexPreviousSelectedSubject = rowIndex;
                    })
                }
            }
        }
    }


    //
    // Create Characteristic Popup
    //
    Popup.CreateCharacteristicPopup {
        id: createCharacteristicPopup

        //anchors.centerIn: parent

        controller: rootItem.controller
    }

}
