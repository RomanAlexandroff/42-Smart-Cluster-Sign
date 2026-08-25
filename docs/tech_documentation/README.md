# Technical documentation

Readable, searchable chapters for the 42 Smart Cluster Sign. This index replaces the old hand-typed table of contents.

To build a single PDF of the whole book, run `make docs` from the repository root (requires `asciidoctor-pdf`). The PDF gets created in `build/Technical_Documentation.pdf` and is automatically gitignored.

## Chapters

1. [Vocabulary of Terms](01-vocabulary-of-terms.adoc) — names used throughout the project: Sign, Intra, Secret, token, Deep Sleep, OTA, SPIFFS.
2. [About the Project](02-about-the-project.adoc) — what the Sign displays and which services it talks to.
3. [Contractor's Requirements](03-contractors-requirements.adoc) — the original requirements and how the finished device matches them.
4. [General Description of the Program Run](04-program-run-overview.adoc) — one exam day, from night sleep through reservation, exam, and back to the cluster number.
5. [Program Run Step-by-Step](05-program-run-step-by-step.adoc) — boot, battery check, cluster-number mode, and where the cycle can end.
6. [How to Build the Sign Yourself](06-how-to-build-the-sign.adoc) — parts list, display pinout, build steps, and hardware alternatives.
7. [Getting Ready to Maintain and Develop the Project](07-development-environment.adoc) — Arduino IDE, libraries, credentials, and USB upload.
8. [Updating the Program Using Cloud-Pull OTA](08-cloud-pull-ota-updates.adoc) — manifest, GitHub Releases, Telegram / button / weekly triggers.
9. [Uploading the Program as Compiled Binary](09-uploading-compiled-binary.adoc) — flashing a `.bin` with `esptool` when USB Arduino upload is not an option.
10. [Firmware Rollback](10-firmware-rollback.adoc) — how a bad OTA image is detected and the previous partition is restored.
11. [Hardware Maintenance](11-hardware-maintenance.adoc) — charging the Sign and taking it off the wall.
12. [Functions Reference](12-functions-reference.adoc) — program files, `config.h`, and function descriptions.
13. [Architectural Decisions Explained](13-architectural-decisions.adoc) — why `setup()` order, `ota.h` placement, display state, and related choices exist.
14. [How to Get Exams Info from Intra](14-intra-api.adoc) — API steps, curl examples, and example server responses.
15. [Exam Simulation](15-exam-simulation.adoc) — the `EXAM_SIMULATION` macro for testing exam mode without a real exam.
16. [Create Your Own Graphics](16-create-your-own-graphics.adoc) — turning artwork into bitmaps the display can draw.
17. [How to Draw on the Display](17-how-to-draw-on-the-display.adoc) — GxEPD2 coordinates, partial windows, and text bounds.
18. [The Intricacies of Time Keeping](18-time-keeping.adoc) — UTC from Intra, time zones, and EU daylight-saving rules.
19. [Service Messages Meaning](19-service-messages.adoc) — what the display and Telegram chat show, and what to do about it.
20. [Libraries and Their Use](20-libraries.adoc) — Arduino IDE / board versions and every library the project depends on.
21. [Bugs and Suggestions How to Fix Them](21-known-bugs-and-fixes.adoc) — known display, flash, and related problems.
22. [New Bugs and Future Development Suggestions](22-reporting-and-suggestions.adoc) — where to file issues and ideas.
23. [Suggestions for Dealing with Confidential Information](23-confidential-information.adoc) — stub; not yet written (see issue #71).
24. [External Information Sources](24-external-sources.adoc) — datasheets, APIs, and other references.

## Chapters not yet written

These topics were listed as `//TO-DO` in the original Word document and are tracked as follow-up issues:

- [Hardware description](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/65)
- [Circuit diagrams and schematics](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/66)
- [Description of the program constants](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/67)
- [Power management](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/68)
- [Safety considerations](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/69)
- [The Don'ts of changing the program](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/70)
- [Suggestions for dealing with confidential information](https://github.com/RomanAlexandroff/42-Smart-Cluster-Sign/issues/71) (chapter 23 above)
