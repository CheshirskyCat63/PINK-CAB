import hashlib
from pathlib import Path
import tempfile
import unittest

from scripts.ci.materialize_local_lfs import (
    POINTER_VERSION,
    lfs_object_path,
    parse_lfs_pointer,
    sha256_file,
)


class LocalLfsMaterializationTests(unittest.TestCase):
    def test_parse_pointer_returns_exact_sha256_oid(self):
        oid = "a" * 64
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "fixture.umap"
            path.write_text(
                f"{POINTER_VERSION}\noid sha256:{oid}\nsize 123\n",
                encoding="utf-8",
            )
            self.assertEqual(parse_lfs_pointer(path), oid)

    def test_binary_file_is_not_mistaken_for_pointer(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "fixture.uasset"
            path.write_bytes(b"\x00\xffUE-ASSET")
            self.assertIsNone(parse_lfs_pointer(path))

    def test_sha256_file_matches_payload(self):
        payload = b"pink-cab-local-lfs"
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "payload.bin"
            path.write_bytes(payload)
            self.assertEqual(
                sha256_file(path),
                hashlib.sha256(payload).hexdigest(),
            )

    def test_lfs_object_path_uses_git_lfs_fanout(self):
        oid = "0123456789abcdef" * 4
        root = Path("repo")
        self.assertEqual(
            lfs_object_path(root, oid),
            root / ".git" / "lfs" / "objects" / "01" / "23" / oid,
        )


if __name__ == "__main__":
    unittest.main()
