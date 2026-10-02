import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'ci' / 'prepare_metaroad.py'
SPEC = importlib.util.spec_from_file_location('prepare_metaroad', SCRIPT)
vendor = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(vendor)


class VendorPinTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = self.root / 'package'
        (self.source / 'Source').mkdir(parents=True)
        (self.source / 'Source/Example.h').write_bytes(b'canonical')
        self.lock = {'files': [{'path': 'Source/Example.h',
                               'sha256': hashlib.sha256(b'canonical').hexdigest()}]}

    def test_clean_package_provisions_and_rechecks_without_overwrite(self):
        repo = self.root / 'repo'
        vendor.provision(repo, self.source, self.lock)
        vendor.provision(repo, self.source, self.lock)
        self.assertEqual((repo/'Plugins/MetaRoad/Source/Example.h').read_bytes(), b'canonical')

    def test_hash_mismatch_fails_before_project_installation(self):
        (self.source / 'Source/Example.h').write_bytes(b'changed')
        repo = self.root / 'repo'
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            vendor.provision(repo, self.source, self.lock)
        self.assertFalse((repo/'Plugins/MetaRoad').exists())

    def test_mixed_version_source_is_rejected(self):
        (self.source / 'Source/OldDuplicate.h').write_bytes(b'old')
        with self.assertRaisesRegex(ValueError, 'mixed installation'):
            vendor.verify(self.source, self.lock)

    def test_existing_local_modification_is_preserved_and_rejected(self):
        repo = self.root / 'repo'
        vendor.provision(repo, self.source, self.lock)
        local = repo/'Plugins/MetaRoad/Source/Example.h'
        local.write_bytes(b'local edit')
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            vendor.provision(repo, self.source, self.lock)
        self.assertEqual(local.read_bytes(), b'local edit')

    def test_path_traversal_is_rejected(self):
        self.lock['files'][0]['path'] = '../outside'
        with self.assertRaisesRegex(ValueError, 'Unsafe'):
            vendor.verify(self.source, self.lock)
