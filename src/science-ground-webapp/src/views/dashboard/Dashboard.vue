<template>
  <v-container
    id="dashboard"
    fluid
    tag="section"
  >
    <v-row>
      <apexchart width="500" type="bar" :options="chartOptions" :series="series"></apexchart>
    </v-row>
  </v-container>
</template>

<script>
  import VueApexCharts from 'vue-apexcharts';
    import Vue from 'vue';

  export default {
    name: 'DashboardDashboard',
    components: { 'apexchart': VueApexCharts},
    data () {
     return{
       chartOptions: {
          chart: {
            id: 'vuechart-example'
          },
          xaxis: {
            categories: [1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998]
          }
        },
        series: [{
          name: 'series-1',
          data: [30, 40, 35, 50, 49, 60, 70, 91]
        }]
     }
    },
    mounted () {
    Vue.axios
      .get('https://api.coindesk.com/v1/bpi/currentprice.json')
      .then(response => (console.log( response)))
    }
  }
</script>
