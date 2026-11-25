# Repository instructions

This repository is one Open-XY firmware extension, `variations`. It is not a
machine and it is not the SDK: it reaches the OP-XY through the
[Open-XY SDK](https://github.com/etcetc000/open-xy) and the builder's catalog,
which pins a tagged commit of this repository.

Read the SDK's [extensions guide](https://github.com/etcetc000/open-xy/blob/staging/docs/extensions.md)
before changing anything here.

## The rules that do not bend

- **Only the public SDK interface.** `<opxy/extension.hpp>`, `<opxy/draw.hpp>`
  and `<opxy/types.hpp>`. This extension never patches firmware, never names a
  stock address and adds nothing to the SDK repository. If it needs something
  the SDK does not have, that is a separate pull request against the SDK's
  `staging` branch, not a workaround here.
- **The callbacks are bounded.** `sequencer_pass` runs once an audio block,
  about 689 times a second, and the page's drawing runs once a frame. No
  allocation, no blocking, no I/O, no logging, no recursion, no unbounded
  loops. Everything this extension owns is fixed-size and declared up front.
- **No platform conditionals, OS headers or assembly** in this repository.
- **Saved state is a published format.** `store.hpp` holds every version the
  extension has ever written and upgrades them on load. A change to the layout
  is a new version number that older bytes still load into; never change what
  an existing version means, and never drop a version's reader.

## Checks

Clone the SDK beside this repository, then from here:

```sh
python ../open-xy/sdk.py test --extension .
```

That builds and runs every host test in `tests/` and the `variations.py`
tool's own tests. It must pass before any commit.

After changing the page's drawing or anything that bounds it, also run the
fuzzers in [tools/fuzz](tools/fuzz): they assert that every frame the page
produces is drawable at all, which the host tests do not.

After changing the page's layout, regenerate its artwork and check it is
current:

```sh
python tools/page_svg.py            # writes variations_page.svg
python tools/page_svg.py --check    # exit 1 if it is not current
```

## House style

- Comments carry the reasoning, the build they came from and the requirement
  that prompted them — but never a person's name. Keep the substance, drop the
  attribution.
- No absolute paths anywhere, including in test and tool includes: they name
  somebody's home directory and only build on one machine.
- Checks in tests use the file's own `require()`, never `assert`.

## Publishing

The builder reaches this extension through the SDK's
`catalog/extensions.json`, which pins a tag and a commit of this repository. A
release is: tests green, the artwork current, a tag, and then a pull request
against the SDK that changes only that one catalog file.
