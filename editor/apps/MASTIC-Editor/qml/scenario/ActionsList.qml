/*
 *	MASTIC Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *
 */

import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3

import I2Quick 1.0

import MASTIC 1.0
import "../theme" as Theme

Item {
    id: rootItem

    anchors.fill: parent

    //--------------------------------
    //
    // Properties
    //
    //--------------------------------

    // Controller associated to our view
    property var controller : null;


    //-----------------------------------------
    //
    // Functions
    //
    //-----------------------------------------

    // Find a root layer by its object name
    function findLayerRootByObjectName(startingObject, layerObjectName)
    {
        var currentObject = startingObject;
        var layerRoot = null;

        while ((currentObject !== null) && (layerRoot == null))
        {
            var index = 0;
            while ((index < currentObject.data.length) && (layerRoot == null))
            {
                if (currentObject.data[index].objectName === layerObjectName)
                {
                    layerRoot = currentObject.data[index];
                }
                index++;
            }

            currentObject = currentObject.parent;
        }

        return layerRoot;
    }



    //--------------------------------
    //
    // Content
    //
    //--------------------------------

    // allowing to deselect selected action
    MouseArea {
        anchors.fill: parent
        onClicked:  {
            if(controller.selectedAction)
            {
                controller.selectedAction = null;
            }
        }
    }

    //
    // List of actions
    //
    ScrollView {
        id : actionsListScrollView

        anchors {
            top: parent.top
            topMargin: 78
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        style: MasticScrollViewStyle {
        }

        // Prevent drag overshoot on Windows
        flickableItem.boundsBehavior: Flickable.OvershootBounds

        // Content of our item
        ListView {
            id: actionsList

            model: controller.actionsList

            delegate: componentActionsListItem

            /*onCurrentIndexChanged: {
            //console.log("onCurrentIndexChanged " + agentsList.currentIndex);
            console.log("onCurrentIndexChanged " + model.get(agentsList.currentIndex).name);
        }
        onCurrentItemChanged: {
            console.log("onCurrentItemChanged " + agentsList.currentItem);
        }*/

            //
            // Transition animations
            //
            add: Transition {
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
                NumberAnimation { property: "scale"; from: 0.0; to: 1.0 }
            }

            displaced: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce }

                // ensure opacity and scale values return to 1.0
                NumberAnimation { property: "opacity"; to: 1.0 }
                NumberAnimation { property: "scale"; to: 1.0 }
            }

            move: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce }

                // ensure opacity and scale values return to 1.0
                NumberAnimation { property: "opacity"; to: 1.0 }
                NumberAnimation { property: "scale"; to: 1.0 }
            }

            remove: Transition {
                // ensure opacity and scale values return to 0.0
                NumberAnimation { property: "opacity"; to: 0.0 }
                NumberAnimation { property: "scale"; to: 0.0 }
            }

        }
    }

    //
    // Header
    //
    Rectangle {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom : actionsListScrollView.top
        }

        color : MasticTheme.selectedTabsBackgroundColor


        Row {
            id: headerRow

            height: btnAddAction.height
            spacing : 8

            anchors {
                top: parent.top
                topMargin: 23
                left: parent.left
                leftMargin: 10
            }

            Button {
                id: btnAddAction
                activeFocusOnPress: true

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                enabled: controller && controller.agentsInMappingList.count > 0
                style : Theme.LabellessSvgButtonStyle {
                    fileCache: MasticTheme.svgFileMASTIC

                    pressedID: releasedID + "-pressed"
                    releasedID: "creernouvelagent"
                    disabledID : releasedID + "-disabled"
                }

                onClicked: {
                    controller.openActionEditor(null);
                    //actionEditor.show();
                }
            }

            Rectangle {
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                height : btnAddAction.height
                width : 1
                color : MasticTheme.blueGreyColor
            }

            Button {
                id: btnImportAction

                enabled : true
                activeFocusOnPress: true

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                style: Theme.LabellessSvgButtonStyle {
                    fileCache: MasticTheme.svgFileMASTIC

                    pressedID: releasedID + "-pressed"
                    releasedID: "importer"
                    disabledID : releasedID + "-disabled"

                }

                onClicked: {
                    if (MasticEditorC.scenarioC) {
                        MasticEditorC.scenarioC.openScenarioFromFile();
                    }
                }
            }

        }

        Row {
            id: headerRow2

            height: btnAddAction.height
            spacing : 8

            anchors {
                verticalCenter: headerRow.verticalCenter
                right : parent.right
                rightMargin: 10
            }


            Button {
                id: btnExportAction

                enabled: visible & (controller.actionsList.count > 0 ? true : false)
                activeFocusOnPress: true

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                style: Theme.LabellessSvgButtonStyle {
                    fileCache: MasticTheme.svgFileMASTIC

                    pressedID: releasedID + "-pressed"
                    releasedID: "exporter"
                    disabledID : releasedID + "-disabled"
                }

                onClicked: {
                    if (MasticEditorC.scenarioC) {
                        MasticEditorC.scenarioC.saveScenarioToSelectedFile();
                    }
                }
            }


            Rectangle {
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                height : btnRemoveAction.height
                width : 1
                color : MasticTheme.blueGreyColor
            }


            Button {
                id: btnRemoveAction

                enabled: false
                activeFocusOnPress: true

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                style: Theme.LabellessSvgButtonStyle {
                    fileCache: MasticTheme.svgFileMASTIC

                    pressedID: releasedID + "-pressed"
                    releasedID: "supprimerplusieurs"
                    disabledID : releasedID + "-disabled"

                }

                onClicked: {
                    console.log("Supprimer Action")
                    // TODO
                }
            }

        }

    }


    //
    // Separator
    //
    Rectangle {
        anchors {
            bottom: actionsListScrollView.top
            left: parent.left
            right: parent.right
        }

        color: MasticTheme.leftPanelBackgroundColor

        height: 1
    }


    //
    // Visual representation of an action in our list
    //
    Component {
        id: componentActionsListItem


        Item {
            id : actionListItem

            width: MasticTheme.leftPanelWidth
            height: 42

            // Not Draggable Action Item
            ActionsListItem {
                id : notDraggableItem

                anchors.fill : parent

                action : model.QtObject
                controller: rootItem.controller

                // visible: mouseArea.drag.active
                actionItemIsHovered : mouseArea.containsMouse
            }

            // Draggable Action Item
            Item {
                id : draggableItem

                height : notDraggableItem.height
                width : notDraggableItem.width

                // Reference to our action that can be used by a DropArea item
                property var action: model.QtObject
                // StartTime of the action when it is dragged in timeline
                property var temporaryStartTime: null;

                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: mouseArea.mouseX
                Drag.hotSpot.y: mouseArea.mouseY
                Drag.keys: ["ActionsListItem"]

                MouseArea {
                    id: mouseArea

                    anchors.fill: draggableItem

                    hoverEnabled: true

                    drag.smoothed: false
                    drag.target: draggableItem
                    cursorShape: (mouseArea.drag.active)? Qt.PointingHandCursor : Qt.OpenHandCursor //Qt.OpenHandCursor

                    onPressed: {
                        if (controller) {
                            if(controller.selectedAction === model.QtObject)
                            {
                                controller.selectedAction = null;
                            } else {
                                controller.selectedAction = model.QtObject;
                            }
                        }

                        // Find our layer and reparent our popup in it
                        draggableItem.parent = rootItem.findLayerRootByObjectName(draggableItem, "overlayLayerDraggableItem");

                        // Compute new position if needed
                        if (draggableItem.parent != null)
                        {
                            var newPosition = actionListItem.mapToItem(parent, 0, 0);
                            draggableItem.x = newPosition.x;
                            draggableItem.y = newPosition.y;
                        }
                    }

                    onPositionChanged: {
                        itemDragged.x = mouseX - 12 - itemDragged.width;
                        itemDragged.y = mouseY - 12 - itemDragged.height;
                    }

                    onReleased: {
                        draggableItem.Drag.drop();

                        //
                        // Reset the position of our draggable item
                        //
                        // - restore our parent if needed
                        draggableItem.parent = actionListItem;

                        // - restore our previous position in parent
                        draggableItem.x = 0;
                        draggableItem.y = 0;
                    }


                    ActionsListItem {
                        height : notDraggableItem.height
                        width : notDraggableItem.width


                        action : model.QtObject
                        controller: rootItem.controller

                        actionItemIsHovered : mouseArea.containsMouse
                        visible: !mouseArea.drag.active
                    }

                    I2CustomRectangle{
                        id : itemDragged
                        height : columnText.height + 8
                        width : nameAction.width + 14
                        color : MasticTheme.darkBlueGreyColor
                        visible: mouseArea.drag.active

                        // - fuzzy radius around our rectangle
                        fuzzyRadius: 2
                        fuzzyColor : MasticTheme.blackColor

                        Column {
                            id : columnText
                            height : temporaryStartTimeAction.visible ?
                                         childrenRect.height
                                       : nameAction.height
                            anchors.centerIn: parent
                            spacing: 6

                            Text {
                                id : nameAction
                                color : MasticTheme.lightGreyColor
                                text : model.name
                                anchors.horizontalCenter: parent.horizontalCenter
                                horizontalAlignment: Text.AlignHCenter

                                font {
                                    family : MasticTheme.textFontFamily
                                    pixelSize: 14
                                }
                            }

                            Text {
                                id : temporaryStartTimeAction
                                visible: text !== ""
                                color : MasticTheme.lightGreyColor
                                text : draggableItem.temporaryStartTime?
                                           draggableItem.temporaryStartTime.toLocaleString(Qt.locale(),"hh:mm:ss.zzz")
                                         : "";

                                anchors.horizontalCenter: parent.horizontalCenter
                                horizontalAlignment: Text.AlignHCenter

                                font {
                                    family : MasticTheme.textFontFamily
                                    pixelSize: 14
                                }
                            }
                        }

                    }

                }

            }
        }
    }





}
