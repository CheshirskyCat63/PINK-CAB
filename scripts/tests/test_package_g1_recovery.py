import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock
import zipfile


SCRIPT = Path(__file__).resolve().parents[1] / 'ci' / 'package_g1_recovery.py'
SPEC = importlib.util.spec_from_file_location('package_g1_recovery', SCRIPT)
packager = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(packager)


class RecoveryArchiveTests(unittest.TestCase):
    def test_archive_bytes_are_independent_of_host_platform_and_input_order(self):
        constructor = zipfile.ZipInfo
        payload = {'z.txt': b'last', 'a.txt': b'first'}
        results = []
        with tempfile.TemporaryDirectory() as folder:
            for platform in (0, 3):
                class HostZipInfo(constructor):
                    def __init__(self, *args, **kwargs):
                        super().__init__(*args, **kwargs)
                        self.create_system = platform
                output = Path(folder) / f'{platform}.zip'
                with mock.patch.object(packager.zipfile, 'ZipInfo', HostZipInfo):
                    packager.write_archive(output, payload)
                results.append(output.read_bytes())
                payload = dict(reversed(list(payload.items())))
            self.assertEqual(results[0], results[1])

    def test_archive_has_no_compressor_dependency_and_preserves_member_bytes(self):
        payload = {'evidence.json': b'{"source":"exact"}\n'}
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / 'recovery.zip'
            with mock.patch.object(packager.zipfile, 'zlib', None):
                packager.write_archive(output, payload)
            with zipfile.ZipFile(output) as archive:
                self.assertIsNone(archive.testzip())
                self.assertEqual(archive.read('evidence.json'), payload['evidence.json'])
                self.assertEqual(archive.getinfo('evidence.json').compress_type, zipfile.ZIP_STORED)
