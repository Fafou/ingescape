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

    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------

    property TasksController taskController: null;
    property TaskM modelM: taskController ? taskController.selectedTask : null;

    property int indexPreviousSelectedDependentVariable: -1;
    property int indexDependentVariableCurrentlyEditing: -1;

    visible: rootItem.modelM


    //--------------------------------
    //
    //
    // Signals
    //
    //
    //--------------------------------

    //
    //signal todo();



    //--------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------

    /**
     * Edit a dependent variable at a row index
     */
    function editDependentVariableAtRowIndex(rowIndex) {

        console.log("QML: Edit the dependent variable at the row index " + rowIndex);

        // Set the index
        rootItem.indexDependentVariableCurrentlyEditing = rowIndex;
    }


    /**
     * Stop the current edition of a dependent variable
     */
    function stopCurrentEditionOfDependentVariable() {
        if (rootItem.indexDependentVariableCurrentlyEditing > -1)
        {
            console.log("QML: Stop the current edition of the dependent variable at the row index " + rootItem.indexDependentVariableCurrentlyEditing);

            // Reset the index
            rootItem.indexDependentVariableCurrentlyEditing = -1;
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
        color: IngeScapeTheme.veryLightGreyColor
    }

    Text {
        id: taskName

        anchors {
            top: parent.top
            topMargin: 34
            left: parent.left
            leftMargin: 26
        }

        text: rootItem.modelM ? "TASK : " + rootItem.modelM.name.toUpperCase() : "..."
        verticalAlignment: Text.AlignVCenter

        color: IngeScapeAssessmentsTheme.regularDarkBlueHeader
        font {
            family: IngeScapeTheme.labelFontFamily
            weight: Font.Black
            pixelSize: 20
        }
    }

    //
    // Independent Variables Panel
    //
    Item {
        id: panelIndependentVariable

        anchors {
            top: taskName.bottom
            topMargin: 18
            left: taskName.left
            right: parent.right
            rightMargin: 27
        }

        height: (parent.height - 18 - taskName.height - 34) / 2

        Text {
            text: "INDEPENDENT VARIABLES"
            anchors {
                left: parent.left
                verticalCenter: newIndeVarButton.verticalCenter
            }

            verticalAlignment: Text.AlignVCenter

            color: IngeScapeAssessmentsTheme.regularDarkBlueHeader
            font {
                family: IngeScapeTheme.labelFontFamily
                weight: Font.Black
                pixelSize: 18
            }
        }

        Button {
            id: newIndeVarButton
            anchors {
                top: parent.top
                right: parent.right
            }

            height: 40
            width: 263

            style: IngeScapeAssessmentsButtonStyle {
                text: "NEW INDEPENDENT VARIABLE"
            }

            onClicked: {
                console.log("QML: New Independent Variable");

                // Open the popup
                createIndependentVariablePopup.open();
            }
        }

        IngeScapeAssessmentsListHeader {
            id: indeVarListHeader
            anchors {
                top: newIndeVarButton.bottom
                topMargin: 12
                left: parent.left
                right: parent.right
            }

            property var headerColumnWidths: [ 229, 336, width - 229 - 336]

            Row {
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: 15
                    right: parent.right
                    bottom: parent.bottom
                }

                Repeater {
                    model: [ "Name", "Description", "Type" ]

                    Text {
                        anchors {
                            verticalCenter: parent.verticalCenter
                        }

                        width: indeVarListHeader.headerColumnWidths[index]

                        text: modelData
                        color: IngeScapeTheme.whiteColor
                        font {
                            family: IngeScapeTheme.labelFontFamily
                            pixelSize: 16
                            weight: Font.Black
                        }
                    }
                }
            }
        }

        Rectangle {
            anchors {
                top: indeVarListHeader.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            color: IngeScapeTheme.whiteColor

            ListView {
                id: indeVarColumn
                anchors.fill: parent

                model: rootItem.experimentation ? rootItem.experimentation.allSubjects : null

                delegate: Rectangle {
                    id: indeVarDelegate

                    height: 40
                    width: indeVarColumn.width

                    // TODO
                }
            }
        }

        Rectangle {
            id: indeVarBottomShadow
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 4
            gradient: Gradient {
                GradientStop { position: 0.0; color: IngeScapeTheme.whiteColor; }
                GradientStop { position: 1.0; color: IngeScapeTheme.darkGreyColor; }
            }
        }




//        TableView {
//            id: tableIndependentVariable

//            anchors {
//                left: parent.left
//                leftMargin: 10
//                right: parent.right
//                rightMargin: 10
//                top: headerIndependentVariable.bottom
//                topMargin: 10
//                bottom: parent.bottom
//                bottomMargin: 10
//            }

//            rowDelegate: Rectangle {
//                width: childrenRect.width
//                height: styleData.selected ? 30 : 20

//                color: styleData.selected ? "lightblue"
//                                          : (styleData.alternate ? "lightgray" : "white")
//            }

//            headerDelegate: Rectangle {
//                height: 30
//                width: parent.width

//                color: "darkgray"

//                Text {
//                    id: txtColumnHeader1

//                    anchors {
//                        fill: parent
//                        leftMargin: (styleData.column === 0) ? 55 : 5
//                    }
//                    verticalAlignment: Text.AlignVCenter
//                    horizontalAlignment: styleData.textAlignment

//                    text: styleData.value
//                    elide: Text.ElideRight
//                    color: IngeScapeTheme.blackColor
//                }

//                Rectangle {
//                    id: leftSeparator1

//                    visible: (styleData.column === 0)

//                    anchors {
//                        left: parent.left
//                        leftMargin: 50
//                        top: parent.top
//                        bottom: parent.bottom
//                    }
//                    width: 1
//                    color: "silver"
//                }

//                Rectangle {
//                    id: rightSeparator1

//                    anchors {
//                        right: parent.right
//                        top: parent.top
//                        bottom: parent.bottom
//                    }
//                    width: 1
//                    color: "silver"
//                }
//            }

//            model: rootItem.modelM ? rootItem.modelM.independentVariables : null

//            TableViewColumn {
//                role: "name"
//                title: qsTr("Name")
//                width: 200

//                delegate: IndependentVariableTableCell {
//                    cellText: styleData.value

//                    // Slot on signal "Delete Independent Variable"
//                    onDeleteIndependentVariable: {
//                        if (rootItem.taskController && model)
//                        {
//                            //console.log("QML: Delete Independent Variable " + model.name);
//                            rootItem.taskController.deleteIndependentVariable(model.QtObject);
//                        }
//                    }
//                }
//            }

//            TableViewColumn {
//                role: "description"
//                title: qsTr("Description")
//                width: 200

//                delegate: IndependentVariableTableCell {
//                    cellText: styleData.value
//                }
//            }

//            TableViewColumn {
//                role: "valueType"
//                title: qsTr("Type")
//                width: 200

//                delegate: IndependentVariableTableCell {
//                    cellText: {
//                        if (tableIndependentVariable.model) {
//                            var entry = tableIndependentVariable.model.get(styleData.row)
//                            if (entry) {
//                                IndependentVariableValueTypes.enumToString(styleData.value) + ((styleData.value === IndependentVariableValueTypes.INDEPENDENT_VARIABLE_ENUM)
//                                                                                               ? " { " + tableIndependentVariable.model.get(styleData.row).enumValues.join(", ") + " }"
//                                                                                               : "" )
//                            } else {
//                                ""
//                            }
//                        }
//                    }

//                }
//            }

//            /*TableViewColumn {
//                role: "enumValues"
//                title: qsTr("")
//            }*/

//            onDoubleClicked: {
//                var independentVariable = tableIndependentVariable.model.get(row);
//                if (independentVariable)
//                {
//                    //console.log("on Double Clicked on row " + row + " (" + independentVariable.name + ")");

//                    // Update the independent variable currently edited
//                    createIndependentVariablePopup.independentVariableCurrentlyEdited = independentVariable;

//                    // Open the popup
//                    createIndependentVariablePopup.open();
//                }
//            }
//        }
    }


    //
    // Dependent Variables Panel
    //
    Item {
        id: panelDependentVariable

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 1
        }
        height: parent.height / 2

        Rectangle {
            id: topSeparator

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: 1

            color: IngeScapeTheme.darkGreyColor
        }

        Row {
            id: headerDependentVariable

            anchors {
                left: parent.left
                leftMargin: 10
                top: parent.top
                topMargin: 5
            }
            height: 30

            spacing: 20

            Text {
                text: "Dependent Variables"

                height: parent.height
                verticalAlignment: Text.AlignVCenter

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    weight: Font.Medium
                    pixelSize: 18
                }
            }

            Button {
                text: "New Dependent Variable"

                height: parent.height

                onClicked: {
                    console.log("QML: New Dependent Variable");

                    if (taskController) {
                        taskController.createNewDependentVariable();
                    }
                }
            }
        }

        TableView {
            id: tableDependentVariable

            anchors {
                left: parent.left
                leftMargin: 10
                right: parent.right
                rightMargin: 10
                top: headerDependentVariable.bottom
                topMargin: 10
                bottom: parent.bottom
                bottomMargin: 10
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
                    id: txtColumnHeader2

                    anchors {
                        fill: parent
                        leftMargin: (styleData.column === 0) ? 126 : 5
                    }
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: styleData.textAlignment

                    text: styleData.value
                    elide: Text.ElideRight
                    color: IngeScapeTheme.blackColor
                }

                Rectangle {
                    id: leftSeparator2

                    visible: (styleData.column === 0)

                    anchors {
                        left: parent.left
                        leftMargin: 121
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: 1
                    color: "silver"
                }

                Rectangle {
                    id: rightSeparator2

                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: 1
                    color: "silver"
                }
            }

            model: rootItem.modelM ? rootItem.modelM.dependentVariables : null

            TableViewColumn {
                id: columnName

                role: "name"
                title: qsTr("Nom")

                width: (tableDependentVariable.width / 5.0) * 2.0

                delegate: VariableTextEditor {

                    variable: model ? model.QtObject : null

                    variableValue: styleData.value

                    //isSelected: styleData.selected

                    isCurrentlyEditing: (rootItem.indexDependentVariableCurrentlyEditing === styleData.row)


                    //
                    // Slots
                    //

                    /*onEditVariable: {
                        // Edit the dependent variable at the row index
                        rootItem.editDependentVariableAtRowIndex(styleData.row);
                    }

                    onStopEditionOfVariable: {
                        // Stop the current edition of the dependent variable
                        rootItem.stopCurrentEditionOfDependentVariable();
                    }*/

                    onVariableValueUpdated: {
                        if (model)
                        {
                            //console.log("QML: on (Dependent) Variable Value Updated for 'Name' " + value);

                            // Update the name
                            model.name = value;
                        }
                    }

                    onDeleteDependentVariable: {
                        if (rootItem.taskController && model)
                        {
                            //console.log("QML: Delete Dependent Variable " + model.name);
                            rootItem.taskController.deleteDependentVariable(model.QtObject);
                        }
                    }
                }
            }

            TableViewColumn {
                id: columnDescription

                role: "description"
                title: qsTr("Description")

                width: tableDependentVariable.width / 5.0

                delegate: VariableTextEditor {

                    variable: model ? model.QtObject : null

                    variableValue: styleData.value

                    //isSelected: styleData.selected

                    isCurrentlyEditing: (rootItem.indexDependentVariableCurrentlyEditing === styleData.row)


                    //
                    // Slots
                    //
                    onVariableValueUpdated: {
                        if (model)
                        {
                            //console.log("QML: on (Dependent) Variable Value Updated for 'Description' " + value);

                            // Update the description
                            model.description = value;
                        }
                    }
                }
            }

            TableViewColumn {
                id: columnAgentName

                role: "agentName"
                title: qsTr("Agent")

                width: tableDependentVariable.width / 5.0

                delegate: VariableComboBoxEditor {

                    variable: model ? model.QtObject : null

                    variableValue: styleData.value

                    //isSelected: styleData.selected

                    isCurrentlyEditing: (rootItem.indexDependentVariableCurrentlyEditing === styleData.row)

                    models: rootItem.modelM ? rootItem.modelM.hashFromAgentNameToSimplifiedAgent.keys : []


                    //
                    // Slots
                    //
                    onVariableValueUpdated: {
                        if (model)
                        {
                            console.log("QML: on (Dependent) Variable Value Updated for 'Agent Name' " + value);

                            // Update the agent name
                            model.agentName = value;
                        }
                    }
                }
            }

            TableViewColumn {
                id: columnOutputName

                role: "outputName"
                title: qsTr("Sortie")

                width: tableDependentVariable.width / 5.0

                delegate: VariableComboBoxEditor {

                    variable: model ? model.QtObject : null

                    variableValue: styleData.value

                    //isSelected: styleData.selected

                    isCurrentlyEditing: (rootItem.indexDependentVariableCurrentlyEditing === styleData.row)

                    models: rootItem.modelM && model && (model.agentName.length > 0) && rootItem.modelM.hashFromAgentNameToSimplifiedAgent.containsKey(model.agentName) ? rootItem.modelM.hashFromAgentNameToSimplifiedAgent.value(model.agentName).outputNamesList
                                                                                                                                                                        : []


                    //
                    // Slots
                    //
                    onVariableValueUpdated: {
                        if (model)
                        {
                            console.log("QML: on (Dependent) Variable Value Updated for 'Output Name' " + value);

                            // Update the output name
                            model.outputName = value;
                        }
                    }
                }
            }

            onDoubleClicked: {
                //console.log("onDoubleClicked " + row);

                // Edit the dependent variable at the row index
                editDependentVariableAtRowIndex(row);
            }

            // NOT Called
            //onSelectionChanged: {
            //}
            // --> Workaround
            Connections {
                target: tableDependentVariable.selection

                onSelectionChanged: {
                    //console.log("onSelectionChanged:");

                    if ((rootItem.indexPreviousSelectedDependentVariable === rootItem.indexDependentVariableCurrentlyEditing)
                            && (rootItem.indexDependentVariableCurrentlyEditing > -1))
                    {
                        // Stop the current edition of the dependent variable
                        stopCurrentEditionOfDependentVariable();
                    }

                    // Reset
                    rootItem.indexPreviousSelectedDependentVariable = -1;

                    tableDependentVariable.selection.forEach(function(rowIndex) {
                        //console.log("row " + rowIndex + " is selected");

                        // Update
                        rootItem.indexPreviousSelectedDependentVariable = rowIndex;
                    })
                }
            }
        }
    }


    //
    // Create Characteristic Popup
    //
    Popup.CreateIndependentVariablePopup {
        id: createIndependentVariablePopup

        //anchors.centerIn: parent

        controller: rootItem.taskController
    }

}
