<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

<script>
</script>

<table>
    <tr>
        <td>
            <h1>Оформление заказа</h1>
        </td>
        <td width="100%"></td>
        <td><img src="/static/images/16x16/del1.gif"
            onclick="delObject($(this));">
        </td>
     </tr>
    <tr>
         <td>
              <table class="object_basket_widget">
                  <tr>
                      <th>№</th>
                      <th>Фото</th>
                      <th>Название</th>
                      <th>Цена</th>
                      <th>Количество</th>
                      <th>Сумма</th>
                  </tr>
              <perl>
                  my $ret         = '';
                  my $object      = getCurObject();
                  my $products    = $object->getNeighsByType('product');
                  my $i           = 1;
                  my $total_count = 0;
                  my $total_summ  = 0;

                  foreach my $product (@$products){
                      my $count = 1;
                      my $price = $product->getPrice() || 0;
                      my $summ  = $price*$count;

                      $ret .= '<tr>';
                      $ret .= "<td>".$i++."</td>";
                      $ret .= '<td>';

                      my $photos = $product->getNeighsByType("photo");
                      foreach my $photo (@$photos){
                          $ret .= getHref(
                              object_id   =>  $product->getId(),
                              title       =>  $photo->renderEmbedded("small")
                          );
                          last;
                      }
                      $ret .= '</td>';

                      $ret .= '<td>';
                      $ret .= getHref(
                          object_id   => $product->getId(),
                          title       => $product->getTitle()
                      );

                      $ret .= '</td>';
                      $ret .= '<td>'.$price.'</td>';
                      $ret .= '<td>'.$count.'</td>';
                      $ret .= '<td>'.$summ.'</td>';

                      $ret .= '</tr>';

                      $total_count += $count;
                      $total_summ  += $summ;
                  }
                  $ret .= '<tr><td><br/></td></tr>';

                  $ret .= '<tr class="object_basket_summ">';
                  $ret .=     '<td>Итого:</td>';
                  $ret .=     '<td></td>';
                  $ret .=     '<td></td>';
                  $ret .=     '<td></td>';
                  $ret .=     '<td>'.$total_count.'</td>';
                  $ret .=     '<td>'.$total_summ.'</td>';
                  $ret .= '</tr>';

                  $ret .= '<tr>';
                  $ret .=     '<td colspan="6"><br/></td>';
                  $ret .= '</tr>';

                  return $ret;
              </perl>
              </table>
        </td>
    </tr>
</table>
</div>

