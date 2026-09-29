import hashlib
import subprocess
from pathlib import Path
import tempfile
import unittest

from scripts.ci.materialize_local_lfs import (
    POINTER_VERSION,
    lfs_object_path,
    is_excluded_from_all_tracked,
    materialize_required,
    parse_lfs_pointer,
    select_all_tracked_paths,
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


    def test_all_tracked_exclusion_is_prefix_scoped(self):
        tracked = {
            "Content/Dev/Authoring/L_PC_L1_MetaRoadAuthoring.umap",
            "Content/World/L1/Road/RoadSurface.uasset",
        }
        selected = select_all_tracked_paths(
            tracked,
            ["Content/Dev/Authoring/"],
        )
        self.assertEqual(
            [path.as_posix() for path in selected],
            ["Content/World/L1/Road/RoadSurface.uasset"],
        )
        self.assertTrue(
            is_excluded_from_all_tracked(
                "Content\\Dev\\Authoring\\L_PC_L1_MetaRoadAuthoring.umap",
                ["Content/Dev/Authoring/"],
            )
        )
        self.assertFalse(
            is_excluded_from_all_tracked(
                "Content/World/L1/Road/RoadSurface.uasset",
                ["Content/Dev/Authoring/"],
            )
        )


    def test_materialize_required_recovers_exact_relative_oid(self):
        payload = b"exact-pink-cab-lfs-fixture"
        oid = hashlib.sha256(payload).hexdigest()
        with tempfile.TemporaryDirectory() as workspace_tmp, tempfile.TemporaryDirectory() as mirror_tmp:
            workspace = Path(workspace_tmp)
            mirror = Path(mirror_tmp)
            relative = Path("Content/Dev/Maps/Fixture.umap")
            target = workspace / relative
            source = mirror / relative
            target.parent.mkdir(parents=True)
            source.parent.mkdir(parents=True)
            target.write_text(
                f"{POINTER_VERSION}\noid sha256:{oid}\nsize {len(payload)}\n",
                encoding="utf-8",
            )
            source.write_bytes(payload)

            materialize_required(workspace, relative, [mirror])

            self.assertEqual(target.read_bytes(), payload)
            self.assertEqual(
                lfs_object_path(workspace, oid).read_bytes(),
                payload,
            )


    def test_recovered_lfs_fixture_remains_git_clean(self):
        payload = b"pink-cab-clean-lfs-fixture"
        with tempfile.TemporaryDirectory() as workspace_tmp, tempfile.TemporaryDirectory() as mirror_tmp:
            workspace = Path(workspace_tmp)
            mirror = Path(mirror_tmp)
            relative = Path("Content/Dev/Maps/CleanFixture.umap")
            target = workspace / relative
            source = mirror / relative
            target.parent.mkdir(parents=True)
            source.parent.mkdir(parents=True)

            def git(*args: str) -> subprocess.CompletedProcess[str]:
                return subprocess.run(
                    ["git", *args],
                    cwd=workspace,
                    text=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    check=True,
                )

            git("init")
            git("config", "user.email", "pinkcab-ci@example.invalid")
            git("config", "user.name", "PINK CAB CI")
            git("lfs", "install", "--local")
            (workspace / ".gitattributes").write_text(
                "*.umap filter=lfs diff=lfs merge=lfs -text\n",
                encoding="utf-8",
            )
            target.write_bytes(payload)
            git("add", ".gitattributes", relative.as_posix())
            git("commit", "-m", "fixture")

            pointer = git("show", f"HEAD:{relative.as_posix()}").stdout
            target.write_text(pointer, encoding="utf-8")
            oid = parse_lfs_pointer(target)
            self.assertIsNotNone(oid)
            cached = lfs_object_path(workspace, oid)
            if cached.exists():
                cached.unlink()
            source.write_bytes(payload)

            materialize_required(workspace, relative, [mirror])

            self.assertEqual(target.read_bytes(), payload)
            status = git("status", "--porcelain", "--", relative.as_posix()).stdout.strip()
            self.assertEqual(status, "", f"materialized LFS fixture dirtied worktree: {status}")



if __name__ == "__main__":
    unittest.main()
