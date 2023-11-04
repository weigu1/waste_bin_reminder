<?php
	header('Content-Type: text/json');

	// include library to ready XLSX
	include 'SimpleXLSX.php';

	// define what file to load
	$IMPORT='2023-DECHETS_Calendrier_Mersch.xlsx';

	// open the file
	if($xlsx=SimpleXLSX::parse($IMPORT)) {
		/*echo '<pre>';
		print_r( $xlsx->sheetNames() );
		print_r( $xlsx->sheetName( $xlsx->activeSheet ) );
		print_r( $xlsx->rows(1) );
		echo '</pre>';
		exit;*/

		// get name of cols of sheet #1 (Tours)
		$rows=$xlsx->rows(1);
		$cols=array();
		foreach($rows[0] as $key=>$value) {
			$cols[$value]=$key;
		}
		//echo '<pre>'.print_r($cols,true).'</pre>';

		$data = array(array());

		for($i=0; $i<count($rows); $i++) {
			if (($rows[$i][$cols['MENAGERS']])=="1") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = 'waste';
			}
			else if (($rows[$i][$cols['ORGANIQUES']])=="1") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = "biomass";
			}
			else if (($rows[$i][$cols['VERRE']])=="1") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = "glas";
			}
			else if (($rows[$i][$cols['PAPIER']])=="1") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = "paper";
			}
			else if (($rows[$i][$cols['VALORLUX']])=="1") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = "valorlux";
			}
			else if (($rows[$i][$cols['ENCOMBRANTS']])=="1(SD)") {
				$data[substr($rows[$i][$cols['DATE']],0,10)] = "bulky_waste";
			}
		}
        $void = array_shift($data);
		ksort($data);
		echo json_encode($data);
	}
	else {
		echo '{"error":"file not found"}';		// error in JSON format
	}
?>
