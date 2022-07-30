<!DOCTYPE html>
<html>
<title>Server-provided Environment variables</title>
<body>
    <table>
        <th><td colspan="2" align="center">Environment Variables</td></th>
        <?php
            foreach (getenv() as $key => $value) {
                echo "<tr><td>$key</td><td>$value</td></tr>\n";
            }
        ?>
    </table>
</body></html>
