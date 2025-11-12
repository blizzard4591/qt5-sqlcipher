#!/usr/bin/php
<?php

function makeBaseUrl($major, $minor, $patch) {
	if ($major != 6) {
		throw new Exception("Error: Unsupported major version: ".$major."!");
	}
	$path = "src/plugins/sqldrivers/sqlite";

	return 'https://raw.githubusercontent.com/qt/qtbase/v'.$major.'.'.$minor.'.'.$patch.'/'.$path;
}

function fetchPage($pageNumber) {
	$url = 'https://api.github.com/repos/qt/qtbase/tags?page='.$pageNumber;
	$ch = curl_init($url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_HEADER, 1);
	curl_setopt($ch, CURLOPT_USERAGENT, 'Qt6-SqlCipher');
	$tagData = curl_exec($ch);
	$responseCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	if ($responseCode != 200) {
		throw new Exception("Error: Response Code was ".$responseCode." for URL '".$url."'!");
	}

	$header_len = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
	$header = substr($tagData, 0, $header_len);
	$body = substr($tagData, $header_len);

	curl_close($ch);

	$headerLines = explode("\n", $header);
	$hasLinkHeaderMatch = false;

	$hasNext = false;
	$nextPageId = -1;
	
	foreach ($headerLines as $line) {
		$line = trim($line);
		// Link: <https://api.github.com/repositories/9880335/tags?page=2>; rel="prev", <https://api.github.com/repositories/9880335/tags?page=1>; rel="first"
		// Link: <https://api.github.com/repositories/9880335/tags?page=1>; rel="prev", <https://api.github.com/repositories/9880335/tags?page=3>; rel="next", <https://api.github.com/repositories/9880335/tags?page=3>; rel="last", <https://api.github.com/repositories/9880335/tags?page=1>; rel="first"
		
		if (preg_match('/Link:\s*(?:<[^?]+\?page=\d+>;\s*rel="prev",\s*)?<([^?]+\?page=(\d+))>;\s*rel="next",\s*<([^?]+\?page=(\d+))>;\s*rel="last"/si', $line, $regs)) {
			$hasNext = ($pageNumber < $regs[4]);
			$nextPageId = $regs[2];
			$hasLinkHeaderMatch = true;
		} else if (preg_match('/Link:\s*<([^?]+\?page=(\d+))>;\s*rel="first",\s*<([^?]+\?page=(\d+))>;\s*rel="prev"/si', $line, $regs)) {
			$hasNext = false;
			$nextPageId = -1;
			$hasLinkHeaderMatch = true;
		} else if (preg_match('/Link:\s*<([^?]+\?page=(\d+))>;\s*rel="prev",\s*<([^?]+\?page=(\d+))>;\s*rel="first"/si', $line, $regs)) {
			$hasNext = false;
			$nextPageId = -1;
			$hasLinkHeaderMatch = true;
		} else {
			continue;
		}	
	}

	if (!$hasLinkHeaderMatch) {
		throw new Exception("Error: Could not extract Link Header from reply!\nHeader was: ".$header);
	}

	return array(
		'hasNext' => $hasNext,
		'nextPageId' => $nextPageId,
		'data' => $body
	);
}

function fetchFile($url) {
	$ch = curl_init($url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch, CURLOPT_USERAGENT, 'Qt6-SqlCipher');
	$data = curl_exec($ch);
	$responseCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	if ($responseCode != 200) {
		throw new Exception("Error: Response Code was ".$responseCode." for URL ".$url."!");
	}
	
	curl_close($ch);
	
	return $data;
}

function extractVersions($decodedJsonData) {
	$result = array();
	foreach ($decodedJsonData as $key => $value) {
		if (preg_match('/^v(\d+)\.(\d+)\.(\d+)$/si', $value->name, $regs)) {
			if ($regs[1] != '6') {
				echo "Warning: Ignoring version ".$regs[1].'.'.$regs[2].'.'.$regs[3]." which has an unsupported major version!\n";
				continue;
			}
			$result[] = array('full' => $regs[1].'.'.$regs[2].'.'.$regs[3], 'major' => $regs[1], 'minor' => $regs[2], 'patch' => $regs[3]);
		}
	}
	return $result;
}

function fetchFiles($versions, $targetPath) {
	foreach ($versions as $version) {
		if (!is_dir($targetPath.'/'.$version['full'])) {
			mkdir($targetPath.'/'.$version['full']);
		}
		
		echo "Fetching files for version ".$version['full']."...\n";
		$baseUrl = makeBaseUrl($version['major'], $version['minor'], $version['patch']);
		$header = fetchFile($baseUrl .'/'. 'qsql_sqlite_p.h');
		$source = fetchFile($baseUrl .'/'. 'qsql_sqlite.cpp');
		
		file_put_contents($targetPath.'/'.$version['full'].'/qsql_sqlite_p.h', $header);
		file_put_contents($targetPath.'/'.$version['full'].'/qsql_sqlite.cpp', $source);
	}
}

$hasNext = true;
$nextPageId = 1;
$versions = array();

while ($hasNext) {
	echo "Fetching Tag page ".$nextPageId."...\n";
	$info = fetchPage($nextPageId);
	$hasNext = $info['hasNext'];
	$nextPageId = $info['nextPageId'];
	
	$decodedJsonData = json_decode($info['data']);
	$versions = array_merge($versions, extractVersions($decodedJsonData));
}

echo "Now fetching files for ".count($versions)." versions of Qt 6.\n";
fetchFiles($versions, '.');

echo "Done.\n";
