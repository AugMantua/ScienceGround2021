<template>
  <v-app>
    <dashboard-core-app-bar />
    <dashboard-core-view />
  </v-app>
</template>

<script>
  export default {
    name: 'DashboardIndex',

    components: {
      DashboardCoreAppBar: () => import('./components/core/AppBar'),

      DashboardCoreView: () => import('./components/core/View'),
    },

    data: () => ({
      expandOnHover: false,
    }),
  }
</script>
