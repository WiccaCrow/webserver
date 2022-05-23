<!DOCTYPE html>
<!-- Contrbuted by GS -->
<html><head></head><body>
<?php
    foreach (getenv() as $key => $value) {
        echo "$key:$value<br>\n";
    }
?>
</body></html>
