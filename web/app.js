"use strict";

const GRID_SIZE = 20;
const CELL_COUNT = GRID_SIZE * GRID_SIZE;
const DEFAULT_START = 2 * GRID_SIZE + 2;
const DEFAULT_DESTINATION = 17 * GRID_SIZE + 17;

const ALGORITHMS = {
    0: {
        name: "Breadth-First Search",
        shortName: "BFS",
        description: "Explores the grid level by level with a queue. On this unit-cost grid, BFS always finds a shortest path.",
        strategy: "Uninformed",
        guarantee: "Shortest path",
    },
    1: {
        name: "Depth-First Search",
        shortName: "DFS",
        description: "Follows one branch deeply before backtracking with a stack. DFS finds a valid route when one exists, but not necessarily the shortest.",
        strategy: "Uninformed",
        guarantee: "Valid path",
    },
    2: {
        name: "Dijkstra's Algorithm",
        shortName: "Dijkstra",
        description: "Expands the cell with the smallest known distance using a priority queue. Nonnegative costs guarantee a shortest path.",
        strategy: "Uniform-cost",
        guarantee: "Shortest path",
    },
    3: {
        name: "A* Search",
        shortName: "A*",
        description: "Combines distance traveled with Manhattan distance to guide the search toward the destination while preserving optimality.",
        strategy: "Heuristic",
        guarantee: "Shortest path",
    },
};

const SPEEDS = {
    1: { label: "Slow", delay: 40 },
    2: { label: "Normal", delay: 14 },
    3: { label: "Fast", delay: 4 },
};

const elements = {
    grid: document.getElementById("pathfinding-grid"),
    coordinateReadout: document.getElementById("coordinate-readout"),
    messageBanner: document.getElementById("message-banner"),
    messageText: document.getElementById("message-text"),
    engineStatus: document.getElementById("engine-status"),
    engineStatusText: document.getElementById("engine-status-text"),
    algorithmSelect: document.getElementById("algorithm-select"),
    algorithmDescription: document.getElementById("algorithm-description"),
    algorithmStrategy: document.getElementById("algorithm-strategy"),
    algorithmGuarantee: document.getElementById("algorithm-guarantee"),
    modeButtons: Array.from(document.querySelectorAll(".mode-button")),
    speedControl: document.getElementById("speed-control"),
    speedOutput: document.getElementById("speed-output"),
    runButton: document.getElementById("run-button"),
    resetButton: document.getElementById("reset-button"),
    clearButton: document.getElementById("clear-button"),
    resultPill: document.getElementById("result-pill"),
    metricAlgorithm: document.getElementById("metric-algorithm"),
    metricExplored: document.getElementById("metric-explored"),
    metricPathLength: document.getElementById("metric-path-length"),
    metricTime: document.getElementById("metric-time"),
};

const state = {
    wasmModule: null,
    startIndex: DEFAULT_START,
    destinationIndex: DEFAULT_DESTINATION,
    walls: new Set(),
    explored: new Set(),
    path: new Set(),
    mode: "wall",
    animationToken: 0,
    isAnimating: false,
    dragActive: false,
    dragAction: "add",
    dragVisited: new Set(),
    focusedIndex: DEFAULT_START,
};

const cellElements = [];
const reducedMotionQuery = window.matchMedia("(prefers-reduced-motion: reduce)");

function rowOf(index) {
    return Math.floor(index / GRID_SIZE);
}

function columnOf(index) {
    return index % GRID_SIZE;
}

function positionText(index) {
    return `row ${rowOf(index)}, column ${columnOf(index)}`;
}

function describeCell(index) {
    const states = [];
    if (index === state.startIndex) states.push("start");
    if (index === state.destinationIndex) states.push("destination");
    if (state.walls.has(index)) states.push("wall");
    if (state.path.has(index)) states.push("final path");
    else if (state.explored.has(index)) states.push("explored");
    const suffix = states.length > 0 ? `, ${states.join(", ")}` : ", open";
    return `Cell ${positionText(index)}${suffix}`;
}

function renderCell(index) {
    const cell = cellElements[index];
    const isStart = index === state.startIndex;
    const isDestination = index === state.destinationIndex;
    cell.classList.toggle("start", isStart);
    cell.classList.toggle("destination", isDestination);
    cell.classList.toggle("wall", state.walls.has(index));
    cell.classList.toggle("explored", state.explored.has(index) && !isStart && !isDestination);
    cell.classList.toggle("path", state.path.has(index) && !isStart && !isDestination);
    cell.textContent = isStart ? "S" : isDestination ? "D" : "";
    cell.setAttribute("aria-label", describeCell(index));
    cell.setAttribute("aria-selected", String(isStart || isDestination));
}

function renderAllCells() {
    for (let index = 0; index < CELL_COUNT; index += 1) {
        renderCell(index);
    }
}

function createGrid() {
    const fragment = document.createDocumentFragment();
    for (let index = 0; index < CELL_COUNT; index += 1) {
        const cell = document.createElement("button");
        cell.type = "button";
        cell.className = "grid-cell";
        cell.dataset.index = String(index);
        cell.setAttribute("role", "gridcell");
        cell.setAttribute("aria-rowindex", String(rowOf(index) + 1));
        cell.setAttribute("aria-colindex", String(columnOf(index) + 1));
        cell.tabIndex = index === state.focusedIndex ? 0 : -1;
        cellElements.push(cell);
        fragment.appendChild(cell);
    }
    elements.grid.appendChild(fragment);
    renderAllCells();
}

function setMessage(message, tone = "neutral") {
    elements.messageText.textContent = message;
    elements.messageBanner.dataset.tone = tone;
}

function setEngineStatus(message, status) {
    elements.engineStatusText.textContent = message;
    elements.engineStatus.dataset.state = status;
}

function updateAlgorithmDetails() {
    const algorithm = ALGORITHMS[elements.algorithmSelect.value];
    elements.algorithmDescription.textContent = algorithm.description;
    elements.algorithmStrategy.textContent = algorithm.strategy;
    elements.algorithmGuarantee.textContent = algorithm.guarantee;
}

function updateSpeedOutput() {
    const speed = SPEEDS[elements.speedControl.value];
    elements.speedOutput.textContent = `${speed.label} · ${speed.delay} ms`;
}

function resetMetrics() {
    elements.metricAlgorithm.textContent = "—";
    elements.metricExplored.textContent = "—";
    elements.metricPathLength.textContent = "—";
    elements.metricTime.textContent = "—";
    elements.resultPill.textContent = "Not run";
    elements.resultPill.dataset.state = "idle";
}

function formatExecutionTime(milliseconds) {
    if (milliseconds === 0) return "0.0000 ms";
    if (milliseconds < 0.0001) return `${milliseconds.toExponential(3)} ms`;
    if (milliseconds < 1) return `${milliseconds.toFixed(4)} ms`;
    return `${milliseconds.toFixed(3)} ms`;
}

function showMetrics(result) {
    const algorithm = ALGORITHMS[elements.algorithmSelect.value];
    elements.metricAlgorithm.textContent = algorithm.name;
    elements.metricExplored.textContent = result.exploredNodes.toLocaleString();
    elements.metricPathLength.textContent = result.found ? result.pathLength.toLocaleString() : "—";
    elements.metricTime.textContent = formatExecutionTime(result.executionTimeMilliseconds);
    elements.resultPill.textContent = result.found ? "Path found" : "No path";
    elements.resultPill.dataset.state = result.found ? "found" : "missing";
}

function clearVisualization({ clearMetrics = true } = {}) {
    const changed = new Set([...state.explored, ...state.path]);
    state.explored.clear();
    state.path.clear();
    changed.forEach(renderCell);
    if (clearMetrics) resetMetrics();
}

function cancelAnimation() {
    state.animationToken += 1;
    state.isAnimating = false;
    updateControlAvailability();
}

function updateControlAvailability() {
    elements.runButton.disabled = !state.wasmModule || state.isAnimating;
    elements.algorithmSelect.disabled = state.isAnimating;
    elements.speedControl.disabled = state.isAnimating;
    elements.modeButtons.forEach((button) => {
        button.disabled = state.isAnimating;
    });
}

function markGridChanged(message) {
    cancelAnimation();
    clearVisualization();
    setMessage(message);
}

function setMode(mode) {
    if (state.isAnimating) return;
    state.mode = mode;
    elements.grid.dataset.mode = mode;
    elements.grid.classList.toggle("is-wall-mode", mode === "wall");
    elements.modeButtons.forEach((button) => {
        const active = button.dataset.mode === mode;
        button.classList.toggle("active", active);
        button.setAttribute("aria-pressed", String(active));
    });
    const messages = {
        wall: "Wall mode: click or drag across cells to draw or erase obstacles.",
        start: "Start mode: choose an open cell for the starting position.",
        destination: "Destination mode: choose an open cell for the destination.",
    };
    setMessage(messages[mode]);
}

function toggleWall(index, action = null) {
    if (index === state.startIndex || index === state.destinationIndex) {
        setMessage("Start and destination cells cannot be converted into walls.", "warning");
        return false;
    }

    const shouldAdd = action === "add" || (action === null && !state.walls.has(index));
    if (shouldAdd) state.walls.add(index);
    else state.walls.delete(index);
    markGridChanged(`${shouldAdd ? "Added" : "Removed"} wall at ${positionText(index)}.`);
    renderCell(index);
    return true;
}

function moveEndpoint(index, endpoint) {
    if (state.walls.has(index)) {
        setMessage("Endpoints must be placed on an open cell.", "warning");
        return;
    }

    const otherIndex = endpoint === "start" ? state.destinationIndex : state.startIndex;
    if (index === otherIndex) {
        setMessage("Start and destination must occupy different cells.", "warning");
        return;
    }

    const previousIndex = endpoint === "start" ? state.startIndex : state.destinationIndex;
    if (previousIndex === index) {
        setMessage(`${endpoint === "start" ? "Start" : "Destination"} is already at ${positionText(index)}.`);
        return;
    }

    if (endpoint === "start") state.startIndex = index;
    else state.destinationIndex = index;
    markGridChanged(`${endpoint === "start" ? "Start" : "Destination"} moved to ${positionText(index)}.`);
    renderCell(previousIndex);
    renderCell(index);
}

function editCell(index) {
    if (state.isAnimating) return;
    if (state.mode === "wall") toggleWall(index);
    else moveEndpoint(index, state.mode);
}

function beginWallDrag(event) {
    if (state.isAnimating || state.mode !== "wall" || event.button !== 0) return;
    const cell = event.target.closest(".grid-cell");
    if (!cell) return;
    event.preventDefault();
    const index = Number(cell.dataset.index);
    state.dragActive = true;
    state.dragAction = state.walls.has(index) ? "remove" : "add";
    state.dragVisited = new Set();
    applyWallDrag(index);
}

function applyWallDrag(index) {
    if (!state.dragActive || state.dragVisited.has(index)) return;
    state.dragVisited.add(index);
    toggleWall(index, state.dragAction);
}

function continueWallDrag(event) {
    if (!state.dragActive) return;
    event.preventDefault();
    const target = document.elementFromPoint(event.clientX, event.clientY);
    const cell = target?.closest(".grid-cell");
    if (cell && elements.grid.contains(cell)) {
        applyWallDrag(Number(cell.dataset.index));
    }
}

function endWallDrag() {
    state.dragActive = false;
    state.dragVisited.clear();
}

function focusCell(index) {
    const boundedIndex = Math.max(0, Math.min(CELL_COUNT - 1, index));
    cellElements[state.focusedIndex].tabIndex = -1;
    state.focusedIndex = boundedIndex;
    cellElements[state.focusedIndex].tabIndex = 0;
    cellElements[state.focusedIndex].focus();
}

function handleGridKeydown(event) {
    const cell = event.target.closest(".grid-cell");
    if (!cell) return;
    const index = Number(cell.dataset.index);
    const row = rowOf(index);
    const column = columnOf(index);
    let nextIndex = null;

    if (event.key === "ArrowUp" && row > 0) nextIndex = index - GRID_SIZE;
    if (event.key === "ArrowDown" && row < GRID_SIZE - 1) nextIndex = index + GRID_SIZE;
    if (event.key === "ArrowLeft" && column > 0) nextIndex = index - 1;
    if (event.key === "ArrowRight" && column < GRID_SIZE - 1) nextIndex = index + 1;
    if (event.key === "Home") nextIndex = row * GRID_SIZE;
    if (event.key === "End") nextIndex = row * GRID_SIZE + GRID_SIZE - 1;

    if (nextIndex !== null) {
        event.preventDefault();
        focusCell(nextIndex);
    }
}

function handleGridClick(event) {
    const cell = event.target.closest(".grid-cell");
    if (!cell || state.isAnimating) return;
    if (state.mode === "wall" && event.detail !== 0) return;
    editCell(Number(cell.dataset.index));
}

function updateCoordinateReadout(event) {
    const cell = event.target.closest(".grid-cell");
    if (!cell) return;
    const index = Number(cell.dataset.index);
    elements.coordinateReadout.textContent = `(${rowOf(index)}, ${columnOf(index)}) · index ${index}`;
}

function copyVector(vector) {
    const values = [];
    try {
        for (let index = 0; index < vector.size(); index += 1) {
            values.push(vector.get(index));
        }
    } finally {
        vector.delete();
    }
    return values;
}

function executeSearch() {
    const blockedVector = new state.wasmModule.IntVector();
    try {
        Array.from(state.walls).sort((left, right) => left - right).forEach((index) => {
            blockedVector.push_back(index);
        });
        const result = state.wasmModule.runPathfinding(
            Number(elements.algorithmSelect.value),
            state.startIndex,
            state.destinationIndex,
            blockedVector
        );
        return {
            validInput: result.validInput,
            errorMessage: result.errorMessage,
            found: result.found,
            explorationOrder: copyVector(result.explorationOrder),
            path: copyVector(result.path),
            pathLength: result.pathLength,
            exploredNodes: result.exploredNodes,
            executionTimeMilliseconds: result.executionTimeMilliseconds,
        };
    } finally {
        blockedVector.delete();
    }
}

function wait(milliseconds) {
    if (milliseconds <= 0) return Promise.resolve();
    return new Promise((resolve) => window.setTimeout(resolve, milliseconds));
}

function animationDelay() {
    return reducedMotionQuery.matches ? 0 : SPEEDS[elements.speedControl.value].delay;
}

async function animateIndices(indices, token, type, delay) {
    for (const index of indices) {
        if (token !== state.animationToken) return false;
        if (index !== state.startIndex && index !== state.destinationIndex) {
            if (type === "explored") state.explored.add(index);
            else state.path.add(index);
            renderCell(index);
        }
        await wait(delay);
    }
    return token === state.animationToken;
}

async function runAlgorithm() {
    if (!state.wasmModule || state.isAnimating) return;

    cancelAnimation();
    clearVisualization();
    state.isAnimating = true;
    const token = state.animationToken;
    updateControlAvailability();
    const algorithm = ALGORITHMS[elements.algorithmSelect.value];
    setMessage(`Running ${algorithm.name} in the C++ WebAssembly engine...`);

    let result;
    try {
        result = executeSearch();
    } catch (error) {
        state.isAnimating = false;
        updateControlAvailability();
        setMessage(`WebAssembly execution failed: ${error instanceof Error ? error.message : String(error)}`, "error");
        return;
    }

    if (!result.validInput) {
        state.isAnimating = false;
        updateControlAvailability();
        setMessage(result.errorMessage || "The C++ engine rejected the grid configuration.", "error");
        return;
    }

    showMetrics(result);
    setMessage(`Animating ${result.exploredNodes} explored nodes from ${algorithm.shortName}...`);
    const delay = animationDelay();
    const explorationCompleted = await animateIndices(result.explorationOrder, token, "explored", delay);
    if (!explorationCompleted) return;

    if (result.found) {
        setMessage(`Path found. Highlighting ${result.pathLength} edges...`, "success");
        const pathCompleted = await animateIndices(result.path, token, "path", Math.max(delay * 1.5, reducedMotionQuery.matches ? 0 : 12));
        if (!pathCompleted) return;
        setMessage(
            `${algorithm.name} found a path with ${result.pathLength} edges after exploring ${result.exploredNodes} nodes.`,
            "success"
        );
    } else {
        setMessage(`${algorithm.name} explored every reachable cell, but no path exists.`, "warning");
    }

    state.isAnimating = false;
    updateControlAvailability();
}

function resetVisualization() {
    cancelAnimation();
    clearVisualization();
    setMessage("Visualization reset. Walls and endpoints were preserved.");
}

function clearGrid() {
    cancelAnimation();
    state.walls.clear();
    state.startIndex = DEFAULT_START;
    state.destinationIndex = DEFAULT_DESTINATION;
    clearVisualization();
    renderAllCells();
    setMessage("Grid cleared. Default endpoints restored.", "success");
}

function bindEvents() {
    elements.modeButtons.forEach((button) => {
        button.addEventListener("click", () => setMode(button.dataset.mode));
    });

    elements.algorithmSelect.addEventListener("change", () => {
        cancelAnimation();
        clearVisualization();
        updateAlgorithmDetails();
        setMessage(`${ALGORITHMS[elements.algorithmSelect.value].name} selected.`);
    });
    elements.speedControl.addEventListener("input", updateSpeedOutput);
    elements.runButton.addEventListener("click", runAlgorithm);
    elements.resetButton.addEventListener("click", resetVisualization);
    elements.clearButton.addEventListener("click", clearGrid);

    elements.grid.addEventListener("pointerdown", beginWallDrag);
    elements.grid.addEventListener("pointermove", continueWallDrag);
    elements.grid.addEventListener("click", handleGridClick);
    elements.grid.addEventListener("keydown", handleGridKeydown);
    elements.grid.addEventListener("pointerover", updateCoordinateReadout);
    elements.grid.addEventListener("focusin", updateCoordinateReadout);
    window.addEventListener("pointerup", endWallDrag);
    window.addEventListener("pointercancel", endWallDrag);
    window.addEventListener("blur", endWallDrag);
}

async function loadWasmModule() {
    if (typeof createPathfindingModule !== "function") {
        throw new Error("Generated module loader not found. Run ./scripts/build_wasm.sh first.");
    }
    return createPathfindingModule({
        locateFile: (fileName) => `../dist/wasm/${fileName}`,
    });
}

async function initialize() {
    createGrid();
    bindEvents();
    updateAlgorithmDetails();
    updateSpeedOutput();
    resetMetrics();
    setMode("wall");
    updateControlAvailability();

    try {
        state.wasmModule = await loadWasmModule();
        setEngineStatus("C++ engine ready", "ready");
        setMessage("WebAssembly loaded. Edit the grid or run the selected algorithm.", "success");
    } catch (error) {
        setEngineStatus("Engine unavailable", "error");
        setMessage(`Could not load WebAssembly: ${error instanceof Error ? error.message : String(error)}`, "error");
    }
    updateControlAvailability();
}

initialize();
