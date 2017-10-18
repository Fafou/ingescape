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

import I2Quick 1.0

import MASTIC 1.0


// agent sub-directory
import "agent" as Agent


Item {
    id: rootItem

    anchors.fill: parent


    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------

    property int popupTopmostZIndex: 1



    //--------------------------------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------------------------------




    //--------------------------------------------------------
    //
    //
    // Content
    //
    //
    //--------------------------------------------------------


    //
    // Center panel
    //
    Item {
        id: centerPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            top: parent.top
            bottom: bottomPanel.top
        }


        AgentsMappingView {
            id: agentsMappingView

            anchors.fill: parent

            controller: MasticEditorC.agentsMappingC
        }
    }


    //
    // Bottom panel
    //
    I2CustomRectangle {
        id: bottomPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            bottom: parent.bottom
        }

        height: MasticTheme.bottomPanelHeight

        color: MasticTheme.scenarioBackgroundColor
        fuzzyRadius: 8


        Text {
            anchors.centerIn: parent

            text: qsTr("Scenario")

            font: MasticTheme.headingFont

            color: MasticTheme.whiteColor
        }
    }



    //
    // Left panel: TODO: add border in I2CustomRectangle
    //
    Rectangle {
        id: leftPanel

        anchors {
            left: parent.left
            top: parent.top
            topMargin: 9
            bottom: parent.bottom
        }

        width: MasticTheme.leftPanelWidth

        color: MasticTheme.leftPanelBackgroundColor
        radius : 5
     //   fuzzyRadius: 8
     //   topRightRadius : 5

        border {
            width: 1
            color: MasticTheme.selectedTabsBackgroundColor
        }

        // tabs of left panel
        I2TabView {
            id : leftPanelTabs

            anchors.fill :parent

            style: I2TabViewStyle {
                frameOverlap: 1
                tab: I2CustomRectangle {
                    color: styleData.selected ? MasticTheme.selectedTabsBackgroundColor : "transparent"
                    implicitWidth: 107
                    implicitHeight: 26
                    topRightRadius : 5

                    Text {
                        id: text
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter;
                        text: styleData.title
                        color: styleData.selected ? MasticTheme.agentsListLabelColor : MasticTheme.selectedTabsBackgroundColor
                        wrapMode: Text.Wrap;

                        font {
                            family: MasticTheme.labelFontFamily;
                            bold: true
                            pixelSize:18;
                            capitalization: Font.AllUppercase;
                        }
                    }
                }
                frame: Rectangle {
                    color: MasticTheme.selectedTabsBackgroundColor
                }
            }

            currentIndex : 0

            onCurrentIndexChanged: {
            }

            Tab {
                title: qsTr("SUPERVISION");
                active : true

                Agent.AgentsList {
                    id: agentsList

                    anchors.fill: parent

                    controller: MasticEditorC.agentsSupervisionC
                }
            }

            Tab {
                title: qsTr("ACTIONS");
                active : false

                Rectangle {
                    id: actions
                    anchors.fill: parent
                    color : "red"
                }
            }

            Tab {
                title: qsTr("RECORDS");
                active : false

                Rectangle {
                    id: records
                    anchors.fill: parent
                    color : "blue"
                }
            }
        }

    }


    // List of "Agent Definition Editor(s)"
    Repeater {
        model: MasticEditorC.modelManager.openedDefinitions

        delegate: Agent.AgentDefinitionEditor {
            id: agentDefinitionEditor

            // Center popup
            x: (parent.width - agentDefinitionEditor.width) / 2.0
            y: (parent.height - agentDefinitionEditor.height) / 2.0

            onOpened: {
                agentDefinitionEditor.z = rootItem.popupTopmostZIndex;
                rootItem.popupTopmostZIndex = rootItem.popupTopmostZIndex + 1;
            }

            onBringToFront: {
                agentDefinitionEditor.z = rootItem.popupTopmostZIndex;
                rootItem.popupTopmostZIndex = rootItem.popupTopmostZIndex + 1;
            }

            onClosed: {
                MasticEditorC.closeDefinition(model.QtObject);
            }
        }
    }


    // AgentMappingHistory ?
}
