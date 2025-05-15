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
    isEditingChoiceText(false),
    connectionRenderMode(ConnectionRenderMode::SINGLE_POINT)
{
    // Initialize the first node as the start node
    nodes.emplace_back(Node{"Start Node", -1, {}, {100, 100}, DragType::SIMPLE, LIGHTGRAY, true});
}

void NodeManager::update()
{
    Vector2 mouse = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();

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
                if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[i].connections.size())) {
                    strncpy(choiceTextBuffer, nodes[i].connections[selectedConnection].choiceText.c_str(), 256);
                } else {
                    choiceTextBuffer[0] = '\0';
                }
                TraceLog(LOG_INFO, "Opened edit UI for node %zu: %s", i, nodes[i].name.c_str());
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

    if (IsKeyPressed(KEY_N))
    {
        addNode(mouse.x - offset.x, mouse.y - offset.y);
    }

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
                size_t inputSlotIndex = getIncomingConnectionIndex(conn.toNodeIndex, i, j);
                Vector2 start = getNodeOutputPos(i, j);
                Vector2 end = getNodeInputPos(conn.toNodeIndex, connectionRenderMode == ConnectionRenderMode::MULTI_POINT ? inputSlotIndex : 0);
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
        float nodeHeight = getNodeHeight(i);
        Vector2 pos = {nodes[i].position.x + offset.x, nodes[i].position.y + offset.y};
        Rectangle rect = {pos.x, pos.y, 150, nodeHeight};
        DrawRectangleRounded(rect, 0.2f, 10, nodes[i].color);
        DrawText(nodes[i].name.c_str(), pos.x + 10, pos.y + 5, 12, BLACK);

        // Draw input/output slots
        if (connectionRenderMode == ConnectionRenderMode::SINGLE_POINT) {
            DrawCircleV(getNodeInputPos(i, 0), 6, BLUE);
            DrawCircleV(getNodeOutputPos(i, 0), 6, RED);
        } else {
            // Draw output slots based on outgoing connections
            size_t numOutConnections = nodes[i].connections.size();
            size_t numOutSlots = numOutConnections > 0 ? numOutConnections : 1;
            for (size_t j = 0; j < numOutSlots; ++j) {
                DrawCircleV(getNodeOutputPos(i, j), 6, RED);
            }
            // Draw input slots based on incoming connections
            size_t numInConnections = getIncomingConnectionCount(i);
            size_t numInSlots = numInConnections > 0 ? numInConnections : 1;
            for (size_t j = 0; j < numInSlots; ++j) {
                DrawCircleV(getNodeInputPos(i, j), 6, BLUE);
            }
        }
    }

    // Draw connection in progress
    if (creatingConnection)
    {
        Vector2 start = getNodeOutputPos(fromNode, 0);
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
    bool wasStartNode = nodes[index].isStartNode;
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
    // If the deleted node was the start node, assign the first node as the new start node
    if (wasStartNode && !nodes.empty()) {
        nodes[0].isStartNode = true;
        TraceLog(LOG_INFO, "Assigned node 0 as new start node after deletion");
    }
}

bool NodeManager::isMouseOverNode(size_t index)
{
    if (index >= nodes.size()) return false;
    float nodeHeight = getNodeHeight(index);
    Vector2 pos = {nodes[index].position.x + offset.x, nodes[index].position.y + offset.y};
    Vector2 size = {150, nodeHeight};
    Vector2 mouse = GetMousePosition();
    return (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y);
}

bool NodeManager::isMouseOverNodeInput(size_t index)
{
    if (index >= nodes.size()) return false;
    Vector2 mouse = GetMousePosition();
    if (connectionRenderMode == ConnectionRenderMode::SINGLE_POINT) {
        Vector2 inputPos = getNodeInputPos(index, 0);
        return CheckCollisionPointCircle(mouse, inputPos, 6);
    } else {
        size_t numInConnections = getIncomingConnectionCount(index);
        size_t numSlots = numInConnections > 0 ? numInConnections : 1;
        for (size_t j = 0; j < numSlots; ++j) {
            Vector2 inputPos = getNodeInputPos(index, j);
            if (CheckCollisionPointCircle(mouse, inputPos, 6)) {
                return true;
            }
        }
        return false;
    }
}

bool NodeManager::isMouseOverNodeOutput(size_t index)
{
    if (index >= nodes.size()) return false;
    Vector2 mouse = GetMousePosition();
    if (connectionRenderMode == ConnectionRenderMode::SINGLE_POINT) {
        Vector2 outputPos = getNodeOutputPos(index, 0);
        return CheckCollisionPointCircle(mouse, outputPos, 6);
    } else {
        size_t numConnections = nodes[index].connections.size();
        size_t numSlots = numConnections > 0 ? numConnections : 1;
        for (size_t j = 0; j < numSlots; ++j) {
            Vector2 outputPos = getNodeOutputPos(index, j);
            if (CheckCollisionPointCircle(mouse, outputPos, 6)) {
                return true;
            }
        }
        return false;
    }
}

bool NodeManager::isMouseOverAnyNode()
{
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (isMouseOverNode(i)) return true;
    }
    return false;
}

size_t NodeManager::getIncomingConnectionCount(size_t nodeIndex)
{
    size_t count = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (const auto& conn : nodes[i].connections) {
            if (conn.toNodeIndex == nodeIndex) {
                ++count;
            }
        }
    }
    return count;
}

size_t NodeManager::getIncomingConnectionIndex(size_t targetNodeIndex, size_t sourceNodeIndex, size_t sourceConnIndex)
{
    size_t index = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes[i].connections.size(); ++j) {
            if (nodes[i].connections[j].toNodeIndex == targetNodeIndex) {
                if (i == sourceNodeIndex && j == sourceConnIndex) {
                    return index;
                }
                ++index;
            }
        }
    }
    TraceLog(LOG_WARNING, "Could not find incoming connection index for target %zu from source %zu, conn %zu", targetNodeIndex, sourceNodeIndex, sourceConnIndex);
    return 0;
}

float NodeManager::getNodeHeight(size_t index)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Invalid node index %zu for getNodeHeight", index);
        return 60.0f;
    }
    if (connectionRenderMode == ConnectionRenderMode::SINGLE_POINT) {
        return 60.0f; // Fixed height in SINGLE_POINT mode
    }
    size_t numInConnections = getIncomingConnectionCount(index);
    size_t numOutConnections = nodes[index].connections.size();
    size_t maxConnections = std::max(numInConnections, numOutConnections);
    size_t numSlots = maxConnections > 0 ? maxConnections : 1;
    // Minimum 20px spacing per slot, plus 20px padding (10px top + 10px bottom)
    float height = std::max(60.0f, static_cast<float>(numSlots) * 20.0f + 20.0f);
    return height;
}

Vector2 NodeManager::getNodeInputPos(size_t index, size_t connectionIndex)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Invalid node index %zu for getNodeInputPos", index);
        return {0, 0};
    }
    float nodeHeight = getNodeHeight(index);
    float x = nodes[index].position.x + offset.x;
    float y = nodes[index].position.y + offset.y + nodeHeight / 2.0f;
    if (connectionRenderMode == ConnectionRenderMode::MULTI_POINT) {
        size_t numInConnections = getIncomingConnectionCount(index);
        size_t numSlots = numInConnections > 0 ? numInConnections : 1;
        float spacing = (nodeHeight - 20.0f) / (numSlots + 1); // 10px padding top/bottom
        y = nodes[index].position.y + offset.y + 10.0f + spacing * (connectionIndex + 1);
    }
    return {x, y};
}

Vector2 NodeManager::getNodeOutputPos(size_t index, size_t connectionIndex)
{
    if (index >= nodes.size()) {
        TraceLog(LOG_WARNING, "Invalid node index %zu for getNodeOutputPos", index);
        return {0, 0};
    }
    float nodeHeight = getNodeHeight(index);
    float x = nodes[index].position.x + offset.x + 150;
    float y = nodes[index].position.y + offset.y + nodeHeight / 2.0f;
    if (connectionRenderMode == ConnectionRenderMode::MULTI_POINT) {
        size_t numConnections = nodes[index].connections.size();
        size_t numSlots = numConnections > 0 ? numConnections : 1;
        float spacing = (nodeHeight - 20.0f) / (numSlots + 1); // 10px padding top/bottom
        y = nodes[index].position.y + offset.y + 10.0f + spacing * (connectionIndex + 1);
    }
    return {x, y};
}

void NodeManager::drawEditUI()
{
    if (selectedNode < 0 || selectedNode >= static_cast<int>(nodes.size())) {
        TraceLog(LOG_WARNING, "Invalid selectedNode %d in drawEditUI", selectedNode);
        editingNode = false;
        selectedNode = -1;
        selectedConnection = -1;
        return;
    }

    strncpy(textBuffer, nodes[selectedNode].name.c_str(), 256);

    // Adjust UI height to accommodate new toggle
    DrawRectangle(600, 50, 180, 560, LIGHTGRAY);
    DrawText("Edit Node", 610, 60, 20, BLACK);

    DrawText("Name:", 610, 90, 12, BLACK);
    if (GuiTextBox({610, 110, 160, 20}, textBuffer, 256, editTextFlag))
    {
        editTextFlag = !editTextFlag;
        TraceLog(LOG_INFO, "Toggled node name edit: %s", textBuffer);
    }

    DrawText("Scene:", 610, 140, 12, BLACK);

    DrawText("Drag Type:", 610, 190, 12, BLACK);

    DrawText("Color:", 610, 240, 12, BLACK);

    // Add Start Node toggle
    DrawText("Start Node:", 610, 290, 12, BLACK);
    bool isStartNode = nodes[selectedNode].isStartNode;
    if (GuiCheckBox({610, 310, 20, 20}, "", &isStartNode))
    {
        if (isStartNode && !nodes[selectedNode].isStartNode) {
            // Clear existing start node
            for (auto& node : nodes) {
                node.isStartNode = false;
            }
            nodes[selectedNode].isStartNode = true;
            TraceLog(LOG_INFO, "Set node %d as start node", selectedNode);
        } else if (!isStartNode && nodes[selectedNode].isStartNode) {
            nodes[selectedNode].isStartNode = false;
            // Assign first node as start node if no other is selected
            bool hasStartNode = false;
            for (const auto& node : nodes) {
                if (node.isStartNode) {
                    hasStartNode = true;
                    break;
                }
            }
            if (!hasStartNode && !nodes.empty()) {
                nodes[0].isStartNode = true;
                TraceLog(LOG_INFO, "Assigned node 0 as start node after unsetting");
            }
        }
    }

    DrawText("Connections:", 610, 340, 12, BLACK);

    if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
        DrawText("Choice Text:", 610, 360, 12, BLACK);
        if (GuiTextBox({610, 380, 160, 20}, choiceTextBuffer, 256, editChoiceTextFlag)) {
            editChoiceTextFlag = !editChoiceTextFlag;
            isEditingChoiceText = editChoiceTextFlag;
            TraceLog(LOG_INFO, "Toggled choice text edit: %s, isEditingChoiceText: %d", choiceTextBuffer, isEditingChoiceText);
        }
    } else {
        isEditingChoiceText = false;
    }

    DrawText("Conn Render:", 610, 410, 12, BLACK);

    if (GuiButton({610, 430, 160, 20}, "Delete Connection") && selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size()))
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

    if (GuiButton({610, 460, 160, 20}, "Save"))
    {
        nodes[selectedNode].name = textBuffer;
        if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
            nodes[selectedNode].connections[selectedConnection].choiceText = choiceTextBuffer;
            TraceLog(LOG_INFO, "Saved choice text for connection %d: %s", selectedConnection, choiceTextBuffer);
        }
        TraceLog(LOG_INFO, "Saved node name: %s", textBuffer);
        isEditingChoiceText = false;
    }

    if (GuiButton({610, 490, 160, 20}, "Delete Node"))
    {
        deleteNode(selectedNode);
        editingNode = false;
        selectedNode = -1;
        selectedConnection = -1;
        isEditingChoiceText = false;
        TraceLog(LOG_INFO, "Node deleted");
    }

    if (GuiButton({610, 520, 160, 20}, "Add New Node"))
    {
        Vector2 mouse = GetMousePosition();
        addNode(mouse.x - offset.x, mouse.y - offset.y);
        TraceLog(LOG_INFO, "Node added at (%f, %f)", mouse.x - offset.x, mouse.y - offset.y);
    }

    if (GuiButton({610, 550, 160, 20}, "Close"))
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

    static int prevSelectedConnection = -1;
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
    if (GuiDropdownBox({610, 350, 160, 20}, connDropdownText.c_str(), &connectionIndex, connDropdownEditMode))
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
        prevSelectedConnection = selectedConnection;
    }

    static bool renderModeDropdownEditMode = false;
    int renderModeIndex = static_cast<int>(connectionRenderMode);
    if (GuiDropdownBox({610, 430, 160, 20}, "Single Point;Multi Point", &renderModeIndex, renderModeDropdownEditMode))
    {
        renderModeDropdownEditMode = !renderModeDropdownEditMode;
        if (!renderModeDropdownEditMode)
        {
            connectionRenderMode = static_cast<ConnectionRenderMode>(renderModeIndex);
            TraceLog(LOG_INFO, "Connection render mode set to: %s", renderModeIndex == 0 ? "Single Point" : "Multi Point");
        }
    }

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
}
