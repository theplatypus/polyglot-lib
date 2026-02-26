from __future__ import annotations

import numpy as np

from .api import MyLibClient


def main() -> None:
    try:
        import polars as pl
    except Exception:
        print("polars not installed; skipping")
        return

    lib = MyLibClient()
    df = pl.DataFrame({"x": [1.0, 2.0, 3.0], "y": [0.5, 1.5, 2.5]})

    x = np.array(df["x"], dtype=np.float64)
    y = np.array(df["y"], dtype=np.float64)
    out = lib.axpy(2.0, x, y)

    result = df.with_columns(pl.Series("axpy", out))
    print(result)


if __name__ == "__main__":
    main()
