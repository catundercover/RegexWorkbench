/** Connects browser input and clipboard facilities to the Emscripten application. */
mergeInto(LibraryManager.library, {
    regex_browser_install_handlers__deps: [
        "$lengthBytesUTF8",
        "$stringToUTF8",
        "$UTF8ToString",
        "malloc",
        "free"
    ],
    /** Installs the browser event handlers exactly once. */
    regex_browser_install_handlers: function() {
        if (Module.regexThesisBrowserHandlersInstalled) {
            return;
        }
        Module.regexThesisBrowserHandlersInstalled = true;

        var canvas = Module.canvas || document.querySelector("canvas");
        if (canvas) {
            canvas.setAttribute("tabindex", "0");
            canvas.style.outline = "none";

            /** Copies selected text for older or non-secure browser contexts. */
            function copyThroughSelection(text) {
                var textArea = document.createElement("textarea");
                textArea.value = text;
                textArea.setAttribute("readonly", "");
                textArea.setAttribute("aria-hidden", "true");
                textArea.style.position = "fixed";
                textArea.style.left = "-10000px";
                textArea.style.top = "0";
                textArea.style.opacity = "0";
                document.body.appendChild(textArea);
                textArea.focus();
                textArea.select();
                textArea.setSelectionRange(0, textArea.value.length);

                try {
                    document.execCommand("copy");
                } catch (error) {
                    // Dear ImGui's in-memory clipboard still supports in-application paste.
                }

                document.body.removeChild(textArea);
                canvas.focus();
            }

            /** Uses the async API when possible and falls back to copying the current selection. */
            Module.regexThesisWriteClipboard = function(text) {
                if (navigator.clipboard && navigator.clipboard.writeText &&
                    window.isSecureContext) {
                    try {
                        navigator.clipboard.writeText(text).catch(function() {
                            copyThroughSelection(text);
                        });
                        return;
                    } catch (error) {
                        // Continue with the synchronous fallback below.
                    }
                }
                copyThroughSelection(text);
            };

            /** Copies a registered ImGui button while the browser still grants user activation. */
            function copyRegisteredTarget(clientX, clientY) {
                var target = Module.regexThesisClipboardTarget;
                if (!target) {
                    return;
                }

                var bounds = canvas.getBoundingClientRect();
                var x = clientX - bounds.left;
                var y = clientY - bounds.top;
                if (x >= target.minimumX && x <= target.maximumX &&
                    y >= target.minimumY && y <= target.maximumY) {
                    Module.regexThesisWriteClipboard(UTF8ToString(target.textPointer));
                }
            }

            canvas.addEventListener("mousedown", function(event) {
                canvas.focus();
                copyRegisteredTarget(event.clientX, event.clientY);
            }, true);
            canvas.addEventListener("touchend", function(event) {
                if (event.changedTouches && event.changedTouches.length > 0) {
                    var touch = event.changedTouches[0];
                    copyRegisteredTarget(touch.clientX, touch.clientY);
                }
            }, true);
        }

        /** Returns whether keyboard input currently belongs to the application canvas. */
        function canvasHasFocus() {
            return canvas && document.activeElement === canvas;
        }

        /** Accounts for browsers that move DOM focus away from the canvas during text input. */
        function applicationOwnsTextInput() {
            return canvasHasFocus() || Module.regexThesisTextInputActive === true;
        }

        /** Copies browser text into WASM memory for one native callback. */
        function sendText(callback, text) {
            if (typeof text !== "string" || text.length === 0) {
                return;
            }
            var length = lengthBytesUTF8(text) + 1;
            var pointer = _malloc(length);
            stringToUTF8(text, pointer, length);
            callback(pointer);
            _free(pointer);
        }

        /** Forwards paste text only while an ImGui text field owns keyboard input. */
        window.addEventListener("paste", function(event) {
            var clipboard = event.clipboardData || window.clipboardData;
            if (!clipboard || !applicationOwnsTextInput()) {
                return;
            }
            var text = clipboard.getData("text/plain");
            if (typeof text === "string" && text.length > 0) {
                sendText(_regex_browser_paste_text, text);
                event.preventDefault();
            }
        }, true);

        /** Preserves browser shortcuts while forwarding the otherwise dead caret key. */
        window.addEventListener("keydown", function(event) {
            var ctrlOrCmd = event.ctrlKey || event.metaKey;
            var key = event.key;
            if (ctrlOrCmd && key !== "^" && !(event.shiftKey && key === "6")) {
                if (key.toLowerCase() === "v") {
                    event.stopImmediatePropagation();
                    return;
                }
                if (key === "+" || key === "-" || key === "=" || key === "0") {
                    event.stopImmediatePropagation();
                }
                return;
            }
            if (!ctrlOrCmd && !event.altKey && canvasHasFocus() &&
                (key === "^" || key === "Dead")) {
                sendText(_regex_browser_text_input, "^");
                event.preventDefault();
                event.stopPropagation();
            }
        }, true);

        /** Prevents the browser menu from competing with Dear ImGui interactions. */
        window.addEventListener("contextmenu", function(event) {
            event.stopImmediatePropagation();
        }, true);
    },

    /** Removes the previous frame's browser-level copy-button hit target. */
    regex_browser_clear_clipboard_target: function() {
        Module.regexThesisClipboardTarget = null;
    },

    /** Mirrors ImGui's text-input ownership for Firefox-compatible paste routing. */
    regex_browser_set_text_input_active: function(active) {
        Module.regexThesisTextInputActive = active !== 0;
    },

    /** Makes one visible ImGui copy button available to the next trusted pointer event. */
    regex_browser_register_clipboard_target: function(
        minimumX, minimumY, maximumX, maximumY, textPointer
    ) {
        Module.regexThesisClipboardTarget = {
            minimumX: minimumX,
            minimumY: minimumY,
            maximumX: maximumX,
            maximumY: maximumY,
            textPointer: textPointer
        };
    },

    regex_browser_write_clipboard__deps: ["$UTF8ToString"],
    /** Mirrors programmatic writes after browser-level pointer handling has already run. */
    regex_browser_write_clipboard: function(textPointer) {
        var text = UTF8ToString(textPointer);
        if (Module.regexThesisWriteClipboard) {
            Module.regexThesisWriteClipboard(text);
        }
    }
});
