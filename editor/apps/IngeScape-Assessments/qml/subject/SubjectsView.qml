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

    property IngeScapeModelManager modelManager: null;

    property ExperimentationM experimentation: modelManager ? modelManager.currentExperimentation : null;



    //--------------------------------
    //
    //
    // Signals
    //
    //
    //--------------------------------

    // Close Subjects view
    signal closeSubjectsView();



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

        color: "#FF333366"
    }

    Button {
        id: btnClose

        anchors {
            right: parent.right
            top: parent.top
        }

        text: "X"

        onClicked: {
            console.log("QML: close Subjects view");

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

        spacing: 20

        Text {
            id: title

            text: "Subjects"

            color: IngeScapeTheme.whiteColor
            font {
                family: IngeScapeTheme.textFontFamily
                weight : Font.Medium
                pixelSize : 20
            }
        }

        Button {
            text: "New Subject"

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
        width: 350

        color: "#44222222"

        Row {
            id: characteristicsHeader

            anchors {
                left: parent.left
                top: parent.top
            }

            spacing: 20

            Text {
                text: "Characteristics"

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    weight : Font.Medium
                    pixelSize : 16
                }
            }

            Button {
                text: "New Characteristic"

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

        color: "#44AAAAAA"



        /*TableView {
            anchors {
                fill: parent
                margins: 10
            }

            /*TableViewColumn {
                role: "uid"
                title: "ID"
                width: 150
            }

            TableViewColumn {
                role: "name"
                title: "Name"
                width: 150
            }

            model: rootItem.experimentation ? rootItem.experimentation.allSubjects : null

        }*/

        Rectangle {
            id: tableHeader

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: 30

            color: "#88222222"
            border {
                color: "black"
                width: 1
            }

            Row {
                anchors.fill: parent

                spacing: 0

                Repeater {
                    model: rootItem.experimentation ? rootItem.experimentation.allCharacteristics : null

                    delegate: Rectangle {
                        id: headerColumn

                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: 150

                        color: "transparent"
                        border {
                            color: "black"
                            width: 1
                        }

                        Text {
                            anchors.centerIn: parent

                            text: model.name

                            color: IngeScapeTheme.whiteColor
                            font {
                                family: IngeScapeTheme.textFontFamily
                                weight : Font.Medium
                                pixelSize : 12
                            }
                        }
                    }
                }
            }
        }

        Column {
            anchors {
                left: parent.left
                right: parent.right
                top: tableHeader.bottom
                bottom: parent.bottom
            }

            Repeater {
                model: rootItem.experimentation ? rootItem.experimentation.allSubjects : null

                delegate: Subject {

                    modelM: model.QtObject
                    allCharacteristics: rootItem.experimentation.allCharacteristics


                    //
                    // Slots
                    //
                    onDeleteSubject: {
                        if (rootItem.controller) {
                            rootItem.controller.deleteSubject(model.QtObject);
                        }
                    }
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
