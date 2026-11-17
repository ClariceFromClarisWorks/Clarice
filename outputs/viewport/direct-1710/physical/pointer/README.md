# Physical pointer anomaly — unresolved trigger

The user reported a boxed-minus mouse pointer after the diagnostic batch.
Screenshot and movement-only capture confirm it is the moving pointer image:
it follows remote mouse movement, with the old position restored. No redraw or
command was needed to remove it from the scrollbar area.

Switching to Finder restored the normal arrow. Returning to Clarice preserved
the arrow; a further scrollbar drag and movement also retained it. No restart,
application update, file operation or production change was used for recovery.

This isolates the visible symptom from retained text/scrollbar pixels. It does
not identify the original cause: Clarice, diagnostics, MultiFinder interactions
and the remote helper have not been isolated. Do not claim a fix or assume memory
corruption. Preserve these captures; instrument cursor state at diagnostic stage
boundaries if it recurs. No per-frame InitCursor workaround was added.
