;;; fct-mode.el --- Major mode for BRS .fct test vector files
;;
;; Installation:
;;
;;   Save this file somewhere in your load-path (for example
;;   ~/.emacs.d/lisp/) and add the following to your init.el:
;;
;;     (require 'fct-mode)
;;
;;   Files ending in ".fct" will then automatically use FCT mode.

(defgroup fct nil
  "Major mode for BRS functional test files."
  :group 'languages)

(defface fct-assignment-keyword-face
  '((t :inherit font-lock-keyword-face))
  "Face used for assignment keywords.")

(defface fct-assignment-value-face
  '((t :inherit font-lock-string-face))
  "Face used for assignment values.")

(defconst fct-assignment-keywords
  '("vector"
    "config"
    "load-low"
    "toggles"
    "load-low-margin"
    "load-high"
    "load-high-margin"
    "load-current"
    "load-current-margin"
    "input-current-margin"
    "input-voltage-high"
    "input-current-high"
    "input-voltage-low"
    "input-current-low"
    "output-drive"
    "output-drive-strength"
    "output-voltage-high"
    "output-voltage-margin"
    )
  "Keywords used on assignment lines.")

(defconst fct-assignment-regexp
  (concat
   "^\\s-*"
   "\\(" (regexp-opt fct-assignment-keywords t) "\\)"
   "\\s-*=\\s-*"
   "\\('\\(?:[^'\n]\\)*'\\)"
   "\\s-*$"))

(defconst fct-font-lock-keywords
  `(
    ;; Whole line comments.
    ("^\\s-*#.*$"
     . font-lock-comment-face)

    ;; Assignment lines.
    (,fct-assignment-regexp
     (1 'fct-assignment-keyword-face)
     (2 'fct-assignment-value-face)))
  "Font lock keywords for FCT mode.")

;;;###autoload
(define-derived-mode fct-mode prog-mode "FCT"
  "Major mode for editing BRS test vector files."

  ;; '#' starts a comment, newline ends it.
  (modify-syntax-entry ?# "<" fct-mode-syntax-table)
  (modify-syntax-entry ?\n ">" fct-mode-syntax-table)

  ;; Comment commands.
  (setq-local comment-start "# ")
  (setq-local comment-end "")

  ;; Font locking.
  (setq-local font-lock-defaults
              '(fct-font-lock-keywords)))

;;;###autoload
(add-to-list 'auto-mode-alist
             '("\\.fct\\'" . fct-mode))

(provide 'fct-mode)
