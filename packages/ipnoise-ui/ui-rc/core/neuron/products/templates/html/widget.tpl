<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronProductsClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
    <h1><perl>
        my $ret = getCurNeuron()->getTitle();
        return $ret;
    </perl></h1>
    </center>

    <perl>
        my $ret             = '';
        my $cur_neuron      = getCurNeuron();
        my $products_els    = getCurNeighsByType("products");
        my $product_els     = getCurNeighsByType("product");
        if (scalar(@$products_els)){
            $ret .= '<div class="neuron_products_list_of_products">';
            $ret .=     '<div class="neuron_products_list_of_products_title">';
            $ret .=         '<h2>Разделы:</h2>';
            $ret .=     '</div>';
            $ret .=     '<div>';
            $ret .=         '<ul class="neuron_products_list_of_products_content">';
            $ret .=             '<table>';
            foreach my $products (@$products_els){
                $ret .= $products->renderEmbedded("widget_tr");
            }
            $ret .=             '</table>';
            $ret .=         '</ul>';
            $ret .=     '</div>';
            $ret .= '</div>';
        }
        if (scalar(@$product_els)){
            $ret .= '<h2>Товары:</h2>';
            $ret .= '<ul>';
            $ret .= '   <table>';
            $ret .= '   <tr style="background: #e6e6e6">';
            $ret .= '       <th>Фото</th>';
            $ret .= '       <th>Наименование</th>';
            $ret .= '   </tr>';
            foreach my $product (@$product_els){
                $ret .= $product->renderEmbedded("widget_tr");
            }
            $ret .= '   </table>';
            $ret .= '</ul>'
        }
        return $ret;
    </perl>
</div>

