//Verifies browser clipboard routing without requiring a graphical browser.
"use strict";

const fs = require("fs");

const bridgePath = process.argv[2];
if (!bridgePath) {
    throw new Error("Expected the browser bridge path as the first argument.");
}

const listeners = {};
const modernWrites = [];
const selectionWrites = [];
const pastedTexts = [];
let selectedTextArea = null;
let nextAllocation = 300;

const canvas = {
    style: {},
    setAttribute: function() {},
    addEventListener: function(name, callback) {
        listeners[name] = callback;
    },
    focus: function() {
        document.activeElement = canvas;
    },
    getBoundingClientRect: function() {
        return {left: 5, top: 7};
    }
};

globalThis.document = {
    activeElement: null,
    querySelector: function(selector) {
        return selector === "canvas" ? canvas : null;
    },
    createElement: function() {
        return {
            value: "",
            style: {},
            setAttribute: function() {},
            focus: function() {
                document.activeElement = this;
            },
            select: function() {
                selectedTextArea = this;
            },
            setSelectionRange: function() {
                selectedTextArea = this;
            }
        };
    },
    execCommand: function(command) {
        if (command !== "copy" || !selectedTextArea) {
            return false;
        }
        selectionWrites.push(selectedTextArea.value);
        return true;
    },
    body: {
        appendChild: function(element) {
            selectedTextArea = element;
        },
        removeChild: function(element) {
            if (selectedTextArea === element) {
                selectedTextArea = null;
            }
        }
    }
};

globalThis.window = {
    isSecureContext: true,
    addEventListener: function(name, callback) {
        listeners["window:" + name] = callback;
    }
};
Object.defineProperty(globalThis, "navigator", {
    configurable: true,
    writable: true,
    value: {
        clipboard: {
            writeText: function(text) {
                modernWrites.push(text);
                return Promise.resolve();
            }
        }
    }
});

globalThis.Module = {canvas: canvas};
globalThis.LibraryManager = {library: {}};
globalThis.mergeInto = function(target, source) {
    Object.assign(target, source);
};
const pointerText = new Map([
    [101, "automaton definition"],
    [202, "rewritten expression"]
]);
globalThis.UTF8ToString = function(pointer) {
    return pointerText.get(pointer) || "";
};
globalThis.lengthBytesUTF8 = function(text) {
    return text.length;
};
globalThis.stringToUTF8 = function(text, pointer) {
    pointerText.set(pointer, text);
};
globalThis._malloc = function() {
    nextAllocation += 1;
    return nextAllocation;
};
globalThis._free = function() {};
globalThis._regex_browser_paste_text = function(pointer) {
    pastedTexts.push(UTF8ToString(pointer));
};
globalThis._regex_browser_text_input = function() {};

const bridgeSource = fs.readFileSync(bridgePath, "utf8");
globalThis.eval(bridgeSource);
const bridge = LibraryManager.library;
bridge.regex_browser_install_handlers();

bridge.regex_browser_clear_clipboard_target();
bridge.regex_browser_register_clipboard_target(10, 10, 80, 40, 101);
listeners.mousedown({clientX: 25, clientY: 27});
if (modernWrites.length !== 1 || modernWrites[0] !== "automaton definition") {
    throw new Error("A registered Copy button did not use the trusted browser pointer event.");
}

bridge.regex_browser_clear_clipboard_target();
listeners.mousedown({clientX: 25, clientY: 27});
if (modernWrites.length !== 1) {
    throw new Error("A cleared Copy-button target remained active.");
}

window.isSecureContext = false;
navigator.clipboard = undefined;
bridge.regex_browser_register_clipboard_target(10, 10, 80, 40, 202);
listeners.mousedown({clientX: 25, clientY: 27});
if (selectionWrites.length !== 1 || selectionWrites[0] !== "rewritten expression") {
    throw new Error("The non-secure Firefox-compatible clipboard fallback was not used.");
}

document.activeElement = null;
bridge.regex_browser_set_text_input_active(1);
let pastePrevented = false;
listeners["window:paste"]({
    clipboardData: {
        getData: function(type) {
            return type === "text/plain" ? "(a|b)^ω" : "";
        }
    },
    preventDefault: function() {
        pastePrevented = true;
    }
});
if (pastedTexts.length !== 1 || pastedTexts[0] !== "(a|b)^ω" || !pastePrevented) {
    throw new Error("Paste text was not forwarded while an ImGui input owned focus.");
}

bridge.regex_browser_set_text_input_active(0);
listeners["window:paste"]({
    clipboardData: {
        getData: function() {
            return "must not be pasted";
        }
    },
    preventDefault: function() {}
});
if (pastedTexts.length !== 1) {
    throw new Error("Paste text was captured without an active ImGui input.");
}

let pasteShortcutStopped = false;
listeners["window:keydown"]({
    ctrlKey: true,
    metaKey: false,
    shiftKey: false,
    altKey: false,
    key: "v",
    stopImmediatePropagation: function() {
        pasteShortcutStopped = true;
    }
});
if (!pasteShortcutStopped) {
    throw new Error("Ctrl+V reached SDL and could suppress the browser paste event.");
}
