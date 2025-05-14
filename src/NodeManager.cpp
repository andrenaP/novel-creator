#include "NodeManager.hpp"

NodeManager::NodeManager(std::vector<Scene>& scenes) :
    scenes(scenes),
    offset{0, 0},
    draggingNode(-1),
    draggingCanvas(false),
    creatingConnection(false),
    selectedNode(-1),
    editingNode(false),
    editTextFlag(false),
    sceneDropdownEditMode(false),
    dragDropdownEditMode(false),
    colorDropdownEditMode(false),
    connDropdownEditMode(false),
    selectedConnection(-1),
    editChoiceTextFlag(false),
    isEditingChoiceText(false) // New flag to track active editing
{
    nodes.emplace_back(Node{"Start Node", -1, {}, {100, 100}, DragType::SIMPLE, LIGHTGRAY});
}

void NodeManager::update()
{
    Vector2 mouse = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();

    // Handle dragging nodes
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        draggingNode = -1;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (isMouseOverNode(i)) {
                draggingNode = static_cast<int>(i);
                break;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
    {
        draggingNode = -1;
    }

    if (draggingNode != -1)
    {
        auto& node = nodes[draggingNode];
        if (node.dragType == DragType::SIMPLE)
        {
            SimpleDrag().move(node.position, mouseDelta);
        } else
        {
            SnappingDrag().move(node.position, mouseDelta);
        }
    }

    // Handle canvas dragging
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isMouseOverAnyNode()) {
        draggingCanvas = true;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        draggingCanvas = false;
        if (creatingConnection) {
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                if (i != fromNode && isMouseOverNodeInput(i))
                {
                    if (i < nodes.size()) {
                        char buffer[256] = "Enter choice text";
                        nodes[fromNode].connections.push_back({i, buffer});
                        TraceLog(LOG_INFO, "Created connection from node %zu to node %zu with choice text: %s", fromNode, i, buffer);
                    } else {
                        TraceLog(LOG_WARNING, "Attempted to create connection to invalid node index %zu", i);
                    }
                    break;
                }
            }
            creatingConnection = false;
            TraceLog(LOG_INFO, "Connection creation ended");
        }
    }

    if (draggingCanvas)
    {
        offset.x += mouseDelta.x;
        offset.y += mouseDelta.y;
    }

    // Handle connection creation
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (isMouseOverNodeOutput(i)) {
                creatingConnection = true;
                fromNode = i;
                TraceLog(LOG_INFO, "Started connection creation from node %zu", i);
                break;
            }
        }
    }

    // Handle node editing
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON))
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (isMouseOverNode(i))
            {
                selectedNode = static_cast<int>(i);
                editingNode = true;
                strncpy(textBuffer, nodes[i].name.c_str(), 256);
                editTextFlag = false;
                sceneDropdownEditMode = false;
                dragDropdownEditMode = false;
                colorDropdownEditMode = false;
                connDropdownEditMode = false;
                selectedConnection = nodes[i].connections.empty() ? -1 : 0;
                editChoiceTextFlag = false;
                isEditingChoiceText = false;
                // Initialize choiceTextBuffer for the first connection (if any)
                if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[i].connections.size())) {
                    strncpy(choiceTextBuffer, nodes[i].connections[selectedConnection].choiceText.c_str(), 256);
                } else {
                    choiceTextBuffer[0] = '\0';
                }
                TraceLog(LOG_INFO, "Opened edit UI for node %zu: %s", i, nodes[i].name.c_str());
                // Log connections for debugging
                TraceLog(LOG_DEBUG, "Node %zu has %zu connections:", i, nodes[i].connections.size());
                for (size_t j = 0; j < nodes[i].connections.size(); ++j) {
                    size_t toNodeIndex = nodes[i].connections[j].toNodeIndex;
                    TraceLog(LOG_DEBUG, "  Connection %zu: toNodeIndex=%zu, choiceText=%s",
                        j, toNodeIndex, nodes[i].connections[j].choiceText.c_str());
                }
                break;
            }
        }
    }

    // Add node with 'N' key
    if (IsKeyPressed(KEY_N))
    {
        addNode(mouse.x - offset.x, mouse.y - offset.y);
    }

    // Delete node with 'DELETE' key when a node is selected
    if (IsKeyPressed(KEY_DELETE) && selectedNode != -1)
    {
        deleteNode(selectedNode);
        selectedNode = -1;
        editingNode = false;
        TraceLog(LOG_INFO, "Deleted node");
    }
}

void NodeManager::draw()
{
    // Draw connections
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        for (size_t j = 0; j < nodes[i].connections.size(); ++j)
        {
            const auto& conn = nodes[i].connections[j];
            if (conn.toNodeIndex < nodes.size()) {
                Vector2 start = getNodeOutputPos(i);
                Vector2 end = getNodeInputPos(conn.toNodeIndex);
                DrawLineBezier(start, end, 2.0f, DARKGRAY);
                DrawText(conn.choiceText.c_str(), (start.x + end.x) / 2, (start.y + end.y) / 2 - 10, 10, BLACK);
            } else {
                TraceLog(LOG_WARNING, "Invalid toNodeIndex %zu in connection %zu for node %zu", conn.toNodeIndex, j, i);
            }
        }
    }

    // Draw nodes
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        Vector2 pos = {nodes[i].position.x + offset.x, nodes[i].position.y + offset.y};
        Rectangle rect = {pos.x, pos.y, 150, 60};
        DrawRectangleRounded(rect, 0.2f, 10, nodes[i].color);
        DrawText(nodes[i].name.c_str(), pos.x + 10, pos.y + 5, 12, BLACK);

        // Draw input/output slots
        DrawCircleV(getNodeInputPos(i), 6, BLUE);
        DrawCircleV(getNodeOutputPos(i), 6, RED);
    }

    // Draw connection in progress
    if (creatingConnection)
    {
        Vector2 start = getNodeOutputPos(fromNode);
        Vector2 end = GetMousePosition();
        DrawLineBezier(start, end, 2.0f, RED);
    }

    // Draw node editor UI
    if (editingNode && selectedNode != -1 && selectedNode < static_cast<int>(nodes.size()))
    {
        drawEditUI();
    }

    // Draw instructions
    DrawText("N: Add Node, DELETE: Remove Selected Node", 10, 580, 10, DARKGRAY);
}

void NodeManager::addNode(float x, float y)
{
    nodes.emplace_back(Node{"Node " + std::to_string(nodes.size() + 1), -1, {}, {x, y}, DragType::SIMPLE, LIGHTGRAY});
    TraceLog(LOG_INFO, "Added node at (%f, %f)", x, y);
}

void NodeManager::deleteNode(size_t index)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Attempted to delete invalid node index %zu", index);
        return;
    }
    nodes.erase(nodes.begin() + index);
    for (auto& node : nodes)
    {
        auto it = node.connections.begin();
        while (it != node.connections.end()) {
            if (it->toNodeIndex == index) {
                it = node.connections.erase(it);
                TraceLog(LOG_INFO, "Removed connection to deleted node %zu", index);
            } else {
                if (it->toNodeIndex > index) {
                    it->toNodeIndex--;
                    TraceLog(LOG_INFO, "Adjusted toNodeIndex to %zu for connection", it->toNodeIndex);
                }
                ++it;
            }
        }
    }
}

bool NodeManager::isMouseOverNode(size_t index)
{
    if (index >= nodes.size()) return false;
    Vector2 pos = {nodes[index].position.x + offset.x, nodes[index].position.y + offset.y};
    Vector2 size = {150, 60};
    Vector2 mouse = GetMousePosition();
    return (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y);
}

bool NodeManager::isMouseOverNodeInput(size_t index)
{
    if (index >= nodes.size()) return false;
    Vector2 inputPos = getNodeInputPos(index);
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointCircle(mouse, inputPos, 6);
}

bool NodeManager::isMouseOverNodeOutput(size_t index)
{
    if (index >= nodes.size()) return false;
    Vector2 outputPos = getNodeOutputPos(index);
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointCircle(mouse, outputPos, 6);
}

bool NodeManager::isMouseOverAnyNode()
{
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (isMouseOverNode(i)) return true;
    }
    return false;
}

Vector2 NodeManager::getNodeInputPos(size_t index)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Invalid node index %zu for getNodeInputPos", index);
        return {0, 0};
    }
    return {
        nodes[index].position.x + offset.x,
        nodes[index].position.y + offset.y + 30
    };
}

Vector2 NodeManager::getNodeOutputPos(size_t index)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Invalid node index %zu for getNodeOutputPos", index);
        return {0, 0};
    }
    return {
        nodes[index].position.x + offset.x + 150,
        nodes[index].position.y + offset.y + 30
    };
}

void NodeManager::drawEditUI()
{
    // Validate selectedNode
    if (selectedNode < 0 || selectedNode >= static_cast<int>(nodes.size())) {
        TraceLog(LOG_WARNING, "Invalid selectedNode %d in drawEditUI", selectedNode);
        editingNode = false;
        selectedNode = -1;
        selectedConnection = -1;
        return;
    }

    // Initialize node name buffer
    strncpy(textBuffer, nodes[selectedNode].name.c_str(), 256);

    // Draw background and static elements first
    DrawRectangle(600, 50, 180, 450, LIGHTGRAY);
    DrawText("Edit Node", 610, 60, 20, BLACK);

    // Node name
    DrawText("Name:", 610, 90, 12, BLACK);
    if (GuiTextBox({610, 110, 160, 20}, textBuffer, 256, editTextFlag))
    {
        editTextFlag = !editTextFlag;
        TraceLog(LOG_INFO, "Toggled node name edit: %s", textBuffer);
    }

    // Scene selection label
    DrawText("Scene:", 610, 140, 12, BLACK);

    // Drag type label
    DrawText("Drag Type:", 610, 190, 12, BLACK);

    // Color selection label
    DrawText("Color:", 610, 240, 12, BLACK);

    // Connection management label
    DrawText("Connections:", 610, 290, 12, BLACK);

    // Choice text editing
    if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
        DrawText("Choice Text:", 610, 340, 12, BLACK);
        if (GuiTextBox({610, 360, 160, 20}, choiceTextBuffer, 256, editChoiceTextFlag)) {
            editChoiceTextFlag = !editChoiceTextFlag;
            isEditingChoiceText = editChoiceTextFlag; // Track active editing
            TraceLog(LOG_INFO, "Toggled choice text edit: %s, isEditingChoiceText: %d", choiceTextBuffer, isEditingChoiceText);
        }
    } else {
        isEditingChoiceText = false; // Reset when no valid connection
    }

    // Buttons
    if (GuiButton({610, 390, 160, 20}, "Delete Connection") && selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size()))
    {
        nodes[selectedNode].connections.erase(nodes[selectedNode].connections.begin() + selectedConnection);
        selectedConnection = nodes[selectedNode].connections.empty() ? -1 : 0;
        choiceTextBuffer[0] = '\0';
        connDropdownEditMode = false;
        isEditingChoiceText = false;
        if (selectedConnection >= 0) {
            strncpy(choiceTextBuffer, nodes[selectedNode].connections[selectedConnection].choiceText.c_str(), 256);
        }
        TraceLog(LOG_INFO, "Connection deleted for node %d, new selectedConnection: %d", selectedNode, selectedConnection);
    }

    if (GuiButton({610, 420, 160, 20}, "Save"))
    {
        nodes[selectedNode].name = textBuffer;
        if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
            nodes[selectedNode].connections[selectedConnection].choiceText = choiceTextBuffer;
            TraceLog(LOG_INFO, "Saved choice text for connection %d: %s", selectedConnection, choiceTextBuffer);
        }
        TraceLog(LOG_INFO, "Saved node name: %s", textBuffer);
        isEditingChoiceText = false; // Reset editing state after save
    }

    if (GuiButton({610, 450, 160, 20}, "Delete Node"))
    {
        deleteNode(selectedNode);
        editingNode = false;
        selectedNode = -1;
        selectedConnection = -1;
        isEditingChoiceText = false;
        TraceLog(LOG_INFO, "Node deleted");
    }

    if (GuiButton({610, 480, 160, 20}, "Add New Node"))
    {
        Vector2 mouse = GetMousePosition();
        addNode(mouse.x - offset.x, mouse.y - offset.y);
        TraceLog(LOG_INFO, "Node added at (%f, %f)", mouse.x - offset.x, mouse.y - offset.y);
    }

    if (GuiButton({610, 510, 160, 20}, "Close"))
    {
        editingNode = false;
        selectedNode = -1;
        selectedConnection = -1;
        sceneDropdownEditMode = false;
        dragDropdownEditMode = false;
        colorDropdownEditMode = false;
        connDropdownEditMode = false;
        editTextFlag = false;
        editChoiceTextFlag = false;
        isEditingChoiceText = false;
        TraceLog(LOG_INFO, "Edit UI closed");
    }

    // Draw dropdowns last to ensure they appear on top

    // Connection dropdown
    static int prevSelectedConnection = -1; // Track previous connection to detect changes
    int connectionIndex = (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) ? selectedConnection : (nodes[selectedNode].connections.empty() ? -1 : 0);
    std::string connDropdownText = "None";
    if (!nodes[selectedNode].connections.empty())
    {
        connDropdownText.clear();
        for (size_t i = 0; i < nodes[selectedNode].connections.size(); ++i)
        {
            size_t toNodeIndex = nodes[selectedNode].connections[i].toNodeIndex;
            if (toNodeIndex < nodes.size()) {
                connDropdownText += "To " + nodes[toNodeIndex].name;
                if (i < nodes[selectedNode].connections.size() - 1) {
                    connDropdownText += ";";
                }
            } else {
                TraceLog(LOG_WARNING, "Invalid toNodeIndex %zu in connection %zu for node %d", toNodeIndex, i, selectedNode);
            }
        }
        if (connDropdownText.empty()) {
            connDropdownText = "None";
            TraceLog(LOG_DEBUG, "No valid connections for node %d", selectedNode);
        }
    } else {
        TraceLog(LOG_DEBUG, "Node %d has no connections", selectedNode);
    }
    TraceLog(LOG_DEBUG, "Connections dropdown text: %s, connectionIndex: %d", connDropdownText.c_str(), connectionIndex);
    if (GuiDropdownBox({610, 310, 160, 20}, connDropdownText.c_str(), &connectionIndex, connDropdownEditMode))
    {
        connDropdownEditMode = !connDropdownEditMode;
        if (!connDropdownEditMode)
        {
            if (connectionIndex >= 0 && connectionIndex < static_cast<int>(nodes[selectedNode].connections.size()))
            {
                selectedConnection = connectionIndex;
                if (!isEditingChoiceText && selectedConnection != prevSelectedConnection) {
                    strncpy(choiceTextBuffer, nodes[selectedNode].connections[selectedConnection].choiceText.c_str(), 256);
                }
                TraceLog(LOG_INFO, "Selected connection %d (To %s) with choice text: %s",
                    selectedConnection, nodes[nodes[selectedNode].connections[selectedConnection].toNodeIndex].name.c_str(), choiceTextBuffer);
            } else
            {
                selectedConnection = nodes[selectedNode].connections.empty() ? -1 : 0;
                if (!isEditingChoiceText && selectedConnection >= 0) {
                    strncpy(choiceTextBuffer, nodes[selectedNode].connections[selectedConnection].choiceText.c_str(), 256);
                } else {
                    choiceTextBuffer[0] = '\0';
                }
                TraceLog(LOG_INFO, "No valid connection selected for node %d, resetting to %d", selectedNode, selectedConnection);
            }
            prevSelectedConnection = selectedConnection;
        }
    } else {
        prevSelectedConnection = selectedConnection; // Update even if dropdown not clicked
    }

    // Color dropdown
    int colorIndex = 0;
    if (nodes[selectedNode].color.r == LIGHTGRAY.r &&
        nodes[selectedNode].color.g == LIGHTGRAY.g &&
        nodes[selectedNode].color.b == LIGHTGRAY.b) {
        colorIndex = 0;
    }
    else if (nodes[selectedNode].color.r == BLUE.r &&
             nodes[selectedNode].color.g == BLUE.g &&
             nodes[selectedNode].color.b == BLUE.b) {
        colorIndex = 1;
    }
    else if (nodes[selectedNode].color.r == GREEN.r &&
             nodes[selectedNode].color.g == GREEN.g &&
             nodes[selectedNode].color.b == GREEN.b) {
        colorIndex = 2;
    }
    else if (nodes[selectedNode].color.r == RED.r &&
             nodes[selectedNode].color.g == RED.g &&
             nodes[selectedNode].color.b == RED.b) {
        colorIndex = 3;
    }
    if (GuiDropdownBox({610, 260, 160, 20}, "Light Gray;Blue;Green;Red", &colorIndex, colorDropdownEditMode))
    {
        colorDropdownEditMode = !colorDropdownEditMode;
        if (!colorDropdownEditMode)
        {
            switch (colorIndex)
            {
                case 0: nodes[selectedNode].color = LIGHTGRAY; break;
                case 1: nodes[selectedNode].color = BLUE; break;
                case 2: nodes[selectedNode].color = GREEN; break;
                case 3: nodes[selectedNode].color = RED; break;
            }
            TraceLog(LOG_INFO, "Color selected: %d", colorIndex);
        }
    }

    // Drag type dropdown
    int dragTypeIndex = static_cast<int>(nodes[selectedNode].dragType);
    if (GuiDropdownBox({610, 210, 160, 20}, "Simple;Snapping", &dragTypeIndex, dragDropdownEditMode))
    {
        dragDropdownEditMode = !dragDropdownEditMode;
        if (!dragDropdownEditMode)
        {
            nodes[selectedNode].dragType = static_cast<DragType>(dragTypeIndex);
            TraceLog(LOG_INFO, "Drag type selected: %d", dragTypeIndex);
        }
    }

    // Scene dropdown
    int selectedScene = nodes[selectedNode].sceneIndex;
    std::string dropdownText = "None";
    if (!scenes.empty()) {
        dropdownText.clear();
        for (size_t i = 0; i < scenes.size(); ++i) {
            dropdownText += scenes[i].name;
            if (i < scenes.size() - 1) dropdownText += ";";
        }
    }
    if (GuiDropdownBox({610, 160, 160, 20}, dropdownText.c_str(), &selectedScene, sceneDropdownEditMode))
    {
        sceneDropdownEditMode = !sceneDropdownEditMode;
        if (!sceneDropdownEditMode && selectedScene >= -1 && selectedScene < static_cast<int>(scenes.size()))
        {
            nodes[selectedNode].sceneIndex = selectedScene;
            TraceLog(LOG_INFO, "Scene selected: %d (%s)", selectedScene,
                selectedScene >= 0 ? scenes[selectedScene].name.c_str() : "None");
        }
    }
}
