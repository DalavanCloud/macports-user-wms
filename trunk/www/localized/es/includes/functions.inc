<?php

# $Id$
# Copyright (c) 2004, OpenDarwin

###############################################################################

# variables and includes
$DOCUMENT_ROOT = $_SERVER['DOCUMENT_ROOT'];
$PHP_SELF = $_SERVER['PHP_SELF'];
include_once("$DOCUMENT_ROOT/includes/db.inc");
$connect = mysql_pconnect($host, $user, $pass) or die("Can't connect to db!");
mysql_select_db($db);

###############################################################################

# create an RSS feed of the project news:

		function create_rss() {
			global $DOCUMENT_ROOT, $connect;
			
			$query = "SELECT id, title, news FROM headlines ORDER BY id DESC LIMIT 10";
			$result = mysql_query($query) or die("Error: $query.");
			if (!$result) {
				die ("Error: $result.");
			}
			
			$open = fopen("$DOCUMENT_ROOT/dp_news.xml", "w+");
			$rssdate = substr_replace(date("Y-m-d\TH:i:sO", time()),':', -2, 0);
			
			$rss = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
			$rss .= "<rss version=\"2.0\"\n";
			$rss .= "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n";
			$rss .= "  xmlns:sy=\"http://purl.org/rss/1.0/modules/syndication/\"\n";
			$rss .= "  xmlns:admin=\"http://webns.net/mvcb/\"\n";
			$rss .= "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n";
			$rss .= "  xmlns:content=\"http://purl.org/rss/1.0/modules/content/\">\n\n";
			
			$rss .= "  <channel>\n";
			$rss .= "	<title>DarwinPorts Project News</title>\n";
			$rss .= "	<link>http://www.darwinports.org/</link>\n";
			$rss .= "	<description>DarwinPorts Project News</description>\n";
			$rss .= "	<dc:language>en-us</dc:language>\n";
			$rss .= "	<dc:creator>Jim Mock (mij@opendarwin.org)</dc:creator>\n";
			$rss .= "	<dc:rights>Copyright 2002-2003</dc:rights>\n";
			$rss .= "	<dc:date>$rssdate</dc:date>\n";
			$rss .= "	<admin:errorReportsTo rdf:resource=\"mailto:mij@opendarwin.org\" />\n";
			
			if (mysql_num_rows($result) > 0) {
				while ($row = mysql_fetch_objects($result)) {
					$rss .= "	<item>\n";
					$rss .= "		<title>$row->title</title>\n";
					$rss .= "		<link>http://www.darwinports.org/es/archives.php?id=$row->id</link>\n";
					
					$desc_query = "SELECT SUBSTRING_INDEX(news, ' ',26) FROM headlines WHERE id=$row->id";
					$desc_result = mysql_query($desc_query);
					$desc_row = mysql_fetch_row($desc_result);
					
					$description = $desc_row[0];
					
					$rss .= "		<description><![CDATA[$description...</p>]]></description>\n";
					$rss .= "		<guid isPermaLink=\"true\">http://www.darwinports.org/es/archives.php?id=$row->id</guid>\n";
					$rss .= "		<content:encoded><![CDATA[$row->news]]></content:encoded>\n";
					$rss .= "	</item>\n";
				}
			}
			$rss .= "	</channel>\n";
			$rss .= "</rss>\n";
			
			$write = fwrite($open, $rss);
			$close = fclose($open);
		}
		
########################################################################

# print the project news:

		function print_headlines() {
			global $connect;
			
			$query = "SELECT id, DATE_FORMAT(timestamp, '%e %b %Y, %l:%i %p') AS f_timestamp, title, news FROM headlines ORDER BY id DESC LIMIT 5";
			$result = mysql_query($query) or die("Error: $query.");
			if (!$result) {
				die("Error: $result.");
			}
			
			if (mysql_num_rows($result) > 0) {
				while ($row = mysql_fetch_object($result)) {
					echo "<h5 class=\"subhdr\">$row->title</h5>\n";
					echo "<h6 class=\"date\">$row->f_timestamp</h6>\n";
					echo "$row->news\n\n";
				}
			} else {
				echo "<p>No hay titulares disponibles en este momento.</p>\n";
			}
		}

########################################################################

# display a single headline:

		function print_headline() {
			global $connect;
			
			$id = $_GET['id'];
			$query = "SELECT id, DATE_FORMAT(timestamp, '%e %b %Y, %l:%i %p') AS f_timestamp, title, news FROM headlines WHERE id='$id'";
			$result = mysql_query($query) or die("Error: $query.");
			if (!result) {
				die("Error: $result.");
			}
			
			$row = mysql_fetch_object($result);
			if ($row) {
				echo "<h5 class=\"subhdr\">$row->title</h5>\n";
				echo "<h6 class=\"date\">$row->f_timestamp</h6>\n";
				echo "$row->news\n\n";
			} else {
				echo "<h2 class=\"hdr\">Error!</h2>\n";
				echo "<p>El titular seleccionado no se pudo encontrar. Puede ser que no se encuentre en la base de datos o que un error ha ocurrido.</p>\n";
			}
		}

########################################################################

# print the form used to add project news

		function print_add_headline() {
			global $PHP_SELF, $connect;
			
			if (!$_POST['submit']) {
				echo "<p>Utilice el formulario que se presenta abajo para agregar noticias del proyecto.</p>\n\n";
				echo "<form action=\"$PHP_SELF\" method=\"post\">\n";
				echo "<p>Título:&#160;&#160;<input type=\"text\" name=\"title\" size=\"40\" /></p>\n";
				echo "<p>Noticias:</p>\n";
				echo "<p><textarea name=\"news\" cols=\"70\" rows=\"20\"></textarea></p>\n";
				echo "<p><input type=\"submit\" name=\"submit\" value=\"Remitir\" /></p>\n";
				echo "</form>\n\n";
			} else {
				$title = $_POST['title'];
				$news = $_POST['news'];
				$errorList = array();
				$count = 0;
				if (!$title) {
					$errorList[$count] = 'Invalid entry: Title';
					$count++;
				}
				if (!$news) {
					$errorList[$count] = 'Invalid entry: News';
					$count++;
				}
				if (sizeof($errorList) == 0) {
					$query = "INSERT INTO headlines (timestamp, title, news) VALUES (NOW(), '$title', '$news')";
					$result = mysql_query($query) or die("Error: $query.");
					if (!$result) {
						die("Error: $result.");
					}
					echo "<p>La entrada fue agregada exitosamente. Puede bien sea <a href=\"list.php\">ver todos los titulares, <a href=\"add.php\">agregar otro titular</a> o <a href=\"/\">volver a la página principal de DarwinPorts</a>.</p>\n\n";
					create_rss();
				} else {
					echo "<p>Han ocurrido los siguientes errores:</p>\n\n";
					echo "<ul>\n";
					for ($x = 0; $x < sizeof($errorList); $x++) {
						echo "<li>$errorList[$x]</li>\n";
					}
					echo "</ul>\n\n";
				}
			}
		}

########################################################################

# print the form used to edit project news:

		function print_edit_headline($id) {
			global $PHP_SELF, $connect;
			
			if (!$_POST['submit']) {
				$id = $_GET['id'];
				$query = "SELECT title, news FROM headlines WHERE id='$id'";
				$result = mysql_query($query) or die("Error: $query.");
				if (!$result) {
					die("Error: $result.");
				}
				if (mysql_num_rows($result) > 0) {
					$row = mysql_fetch_object($result);
					echo "<p>Use el formulario abajo mostrado para editar las noticias del proyecto.</p>\n\n";
					echo "<form action=\"$PHP_SELF?id=$id\" method=\"post\">\n";
					echo "<p>Título:&#160;&#160;<input type=\"text\" name=\"title\" size=\"40\" value=\"$row->title\" /></p>\n";
					echo "<p>Noticias:</p>\n";
					echo "<p><textarea name=\"news\" cols=\"70\" rows=\"20\">";
					echo htmlentities($row->news);
					echo "</textarea></p>\n";
					echo "<p><input type=\"submit\" name=\"submit\" value=\"Editar\" /></p>\n";
					echo "</form>\n\n";
				} else {
					echo "<p>El tilular buscado no se pudo encontrar. Puede ser que no se encuentre en la base de datos o que un error ha ocurrido. Por vafor intente de nuevo.</p>\n\n";
				}
			} else {
				$title = $_POST['title'];
				$news = $_POST['news'];
				$errorList = array();
				$count = 0;
				if (!$title) {
					$errorList[$count] = 'Invalid entry: Title';
					$count++;
				}
				if (!$news) {
					$errorList[$count] = 'Invalid entry: News';
					$count++;
				}
				if (sizeof($errorList) == 0) {
					$query = "UPDATE headlines SET title='$title', news='$news' WHERE id='$id'";
					$result = mysql_query($query) or die("Error: $query.");
					if (!$result) {
						die("Error: $result.");
					}
					echo "<p>El update fue exitoso. Puede bien sea <a href=\"list.php\">ver todos los titulares, <a href=\"add.php\">agregar otro titular</a> o <a href=\"/\">volver a la página principal de DarwinPorts</a>.</p>\n\n";
					create_rss();
				} else {
					echo "<p>Han ocurrido los siguientes errores:</p>\n\n";
					echo "<ul>\n";
					for ($x = 0; $x < sizeof($errorList); $x++) {
						echo "<li>$errorList[$x]</li>\n";
					}
					echo "</ul>\n\n";
				}
			}	
		}

########################################################################

# print a list of all existing headlines:

		function print_all_headlines() {
			global $connect;
			
			echo "<p>Abajo se muestra una lista de todos los titulares existentes. Pueden ser leidos, editados o borrados desde esta interface. Además, también puede <a href=\"add.php\">agregar noticias al proyecto</a>.</p>\n\n";
			$query = "SELECT id, title, DATE_FORMAT(timestamp, '%e %b %Y, %l:%i %p') AS f_timestamp FROM headlines ORDER BY id DESC";
			$result = mysql_query($query) or die("Error: $query.");
			if (!$result) {
				die("Error: $result.");
			}
			if (mysql_num_rows($result) > 0) {
				while ($row = mysql_fetch_object($result)) {
					echo "<p><a href=\"/es/archives.php?id=$row->id.php\">$row->title</a><br />&#160;&#160;$row->f_timestamp&#160;&#160;|&#160;&#160;<a href=\"edit.php?id=$row->id\">editar</a></p>\n\n";
				}
			} else {
				echo "<p>No hay titulares disponibles.</p>\n\n";
			}
		}

######################################################################

# print a list of all existing headlines wihtout the admin foobage
# (this is cheap and could be better)
function print_all_headlines_nonadmin() {

	 global $connect;

	 $query = "SELECT id, title, DATE_FORMAT(timestamp, '%e %b %Y, %l:%i %p') AS f_timestamp FROM headlines ORDER BY id DESC";
	 $result = mysql_query($query) or die("Error: $query.");
	 if (!$result) {
	    die("Error: $result.");
	 }
	 if (mysql_num_rows($result)>0) {
	    while ($row = mysql_fetch_object($result)) {
		  echo "<p><a href=\"/es/archives.php?id=$row->id\">$row->title</a><br />&#160;&#160;$row->f_timestamp</p>\n\n";
		}
	} else {
		  echo "<p>No hay titulares disponibles.</p>\n\n";
	}
}

######################################################################

?>
