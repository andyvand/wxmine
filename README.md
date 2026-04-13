# Minesweeper (wxWidgets port)

A portable wxWidgets / C++17 port of the classic Windows Minesweeper that
lives in the parent directory. Builds on Linux, macOS, and Windows from a
single tree of code, using XRC for the menu and dialogs and an embedded
sprite sheet (PNG) for the board graphics.

## Building

You need wxWidgets 3.x development files and CMake 3.14+.

```sh
# macOS
brew install wxwidgets cmake

# Debian / Ubuntu
sudo apt install libwxgtk3.2-dev cmake build-essential
```

Then:

```sh
cd wx
mkdir build && cd build
cmake ..
cmake --build .
./wxmine        # or open wxmine.app on macOS
```

## How to play

- **Left-click** an unrevealed cell to reveal it. If it's a mine, you lose.
  Otherwise the cell shows the number of mines in the eight neighbouring
  cells (or expands a region of empty cells if the count is 0).
- **Right-click** an unrevealed cell to flag it as a suspected mine. Click
  again to switch to a "?" question-mark (if Marks is enabled) or to clear.
- **Middle-click** (or **left+right together**) on a revealed numbered cell
  to "chord" — reveal all unflagged neighbours at once. Only safe if the
  number of flags around the cell matches the number on it.
- **Click the smiley** to start a new game.

The first cell you reveal is always safe — if you happen to click on a mine,
it's relocated to the first available empty cell.

## Difficulty levels

| Level        | Width × Height | Mines |
| ------------ | -------------- | ----- |
| Beginner     |   9 ×  9       |  10   |
| Intermediate |  16 × 16       |  40   |
| Expert       |  30 × 16       |  99   |
| Custom       |  9–30 × 9–24   | 10–999 (capped at (W-1)×(H-1)) |

## Keyboard

- **F2** — start a new game.

## Menu options

- **Game → Marks (?)** — toggles the third right-click state ("?") on/off.
- **Game → Color** — toggles between the colour and black-and-white sprite
  sheets (preserved for nostalgia).
- **Game → Sound** — toggles the tick / win / lose sound effects.
- **Game → Best Times…** — shows the persistent high-score table.

## Persistence

Settings and best times are stored via `wxConfig`, which writes to the
platform-native location:

- **Windows:** `HKCU\Software\wxmine\wxmine`
- **Linux:**   `~/.wxmine`
- **macOS:**   `~/Library/Preferences/wxmine Preferences`

## Credits

Original Minesweeper by Robert Donner and Curt Johnson.
This port reuses the original sprite art and game algorithm.
