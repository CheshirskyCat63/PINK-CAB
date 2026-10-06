# PINK CAB · Delivery Entry Point

The legacy fast-delivery system is no longer part of the active project.

The only active delivery implementation is:

`scripts/deliver.ps1`

The only active GitHub delivery workflow is:

`.github/workflows/deliver.yml`

The delivery contract is intentionally simple:

1. exact clean source HEAD;
2. `PinkCabEditor` build;
3. focused playable runtime acceptance;
4. BuildCookRun package;
5. packaged executable smoke;
6. atomic desktop publication to `PINCKCAB_BUILD`;
7. `PINCKCAB.lnk` updated only after all previous steps pass.

Old CD-648/CD-869 delivery workflows and `scripts/fast-delivery.ps1` are historical evidence only. They must not be recreated as parallel delivery paths.
