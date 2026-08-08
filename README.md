# Games in C: A Roguelike 

This repository was created for the [RoguelikeDev Does The Complete Roguelike Tutorial (2026)](https://www.reddit.com/r/roguelikedev/comments/1vd9noj/roguelikedev_does_the_complete_roguelike_tutorial/).
If you would like to compile this code, see [these instructions](BUILDING.md).
If you would like to play the game, check it out on [itch.io](https://professor-mario.itch.io/roguelikedev-2026) (you can play it in the browser).

## Assets

The assets used in this game are not bundled in this repository.
If you are attempting to compile and run this repository, then you need to download the files from the creator yourself.
The files you download then need to be placed in an `assets/` folder.

This game uses the [DINOBYTE bitmap font](https://mby.itch.io/dinobyte) by [mby](https://mby.itch.io/).
It is licensed under [Creative Commons Attribution 4.0](http://creativecommons.org/licenses/by/4.0/).
Download the file `dbyte_1x.png` from the creator and save it in the `assets/` directory.

The expected directory setup is:

```
roguelikedev-2026/            (this repo)
├── assets/
│   ├── dbyte_1x.png          (bitmap font not included in the repo)
├── src/
└── CMakeLists.txt
```

