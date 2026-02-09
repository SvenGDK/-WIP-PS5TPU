# PS5TrophyUnlocker
Old test for a PS5 Trophy unlocker.</br>
Previous Goal: While in game, send the payload with user & trophy id to unlock it.

## Payload-way not working
- While testing many modules I noticed that the NpUniversalDataSystem is one of the modules that will not properly loaded in a payload (direct lib calls, dlopen & dlsym)

## What could work
- The old PS4 way does it differently and packages the "Unlocker" as real application in a fPKG
- Going this way on the PS5 would require fPKG support first ...
  - The included homebrew could access NpUniversalDataSystem and interact with the included uds00.ucp & trophy00.ucp
