<?php
for ($i = 1; $i <= 150; $i++)
{
  if ($i % 2 != 0)
  {
    `cp full/$i.png training`;
  }
  else
  {
    `cp full/webcam_$i.png testing/`;
  }
}
?>
